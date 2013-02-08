



#include <utility>
#include <set>
#include <list>
#include <fstream>
#include <iostream>
using namespace std;

#include <boost/filesystem.hpp>
using namespace boost::filesystem;
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
using namespace boost;
#include <exiv2/exiv2.hpp>

#include "Stats.h"
#include "CONFIGURE.h"

typedef set<path> pathset;
typedef list<path> pathlist;


class Jobs : public mutex {
public:
	Jobs (pathset& images) {
		this->images = &images;
		stop = false;
	}
	bool getJob (path& image) {
		lock();
		bool stop = this->stop;
		if (images->empty()) stop = true;
		else {
			pathset::iterator i = images->begin();
			image = *i;
			images->erase(i);
		}
		unlock();
		return !stop;
	}
	void announceStop () {
		lock();
		stop = true;
		unlock();
	}
private:
	pathset* images;
	bool stop;
};


mutex term;
mutex failed_l;

void worker(Jobs* jobs, int id, pathset* failed) {
	path image, thumb;
	while (jobs->getJob(image)) {
		thumb = image;
		thumb.remove_filename();
		thumb += ("/." + image.filename().string() + ".jpg");
		
		std::string key("Exif.Photo.UserComment");
		std::string value = lexical_cast<string>(THUMB_W336_Q80) + lexical_cast<string>(last_write_time(image));
		
		// See if the thumb needs to be (re)created
		if (is_regular_file(thumb)) {
			try {
				Exiv2::Image::AutoPtr exiv_image = Exiv2::ImageFactory::open(thumb.string());
				if (exiv_image.get() != 0) {
					exiv_image->readMetadata();
					Exiv2::ExifData& exifData = exiv_image->exifData();
					if (exifData[key].value().toString() == value) {
						
						continue;
					}
				}
			} catch (Exiv2::Error& e) {}
		}
		
		// Didn't pass all the checks - (re)create it
		term.lock();
		cerr << "[Thumbnail]  " << image.string() << endl;
		term.unlock();
		
		// Make the thumb
		string command = "convert \"" + image.string() + "[0]\" -strip -colorspace sRGB -resize 336x1024 -quality 80 \"" + thumb.string() + "\" &> /dev/null";
		int error = ::system(command.c_str());
		if (error != 0) {
			cerr << "\tconvert: Returned error code: " << error << endl;
			failed_l.lock();
			failed->insert(image);
			failed_l.unlock();
		}
		if (error == 2) {
			cerr << "\tStopping" << endl;
			jobs->announceStop();
			break;
		}
		
		// Write the thumb type and source modify date
		if (error == 0) {
			try {
				Exiv2::Image::AutoPtr newimage = Exiv2::ImageFactory::open(thumb.string());
				if (newimage.get() != 0) {
					newimage->readMetadata();
					Exiv2::ExifData &exifData = newimage->exifData();
					exifData[key] = value;
					newimage->writeMetadata();
				}
			} catch (Exiv2::Error& e) { cerr << "\tProblem writing exif data" << endl; }
		}
	}
}


void process (Stats& stats, pathset& images, pathset& failed) {
	
	int cpu_n = min(32, max(1, int(thread::hardware_concurrency())));
	cerr << "Using " << cpu_n << " threads" << endl;
	
	Jobs jobs(images);
	
	typedef list<thread*> threadlist;
	threadlist threads;
	for (int id=1; id<=cpu_n; id++) {
		threads.push_back(new thread(worker, &jobs, id, &failed));
	}
	for (threadlist::iterator it=threads.begin(); it!=threads.end(); it++) {
		(*it)->join();
		delete (*it);
	}
}









