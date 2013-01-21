
#include <list>
#include <set>
#include <iostream>
#include <fstream>
using namespace std;

#include <boost/filesystem.hpp>
using namespace boost::filesystem;

#include "Stats.h"

typedef list<string> strings;
typedef list<path> pathlist;
typedef set<path> pathset;


// Forward declaration so we don't need a header
void doFolder (Stats&, const path&, pathset& images);
void process (Stats& stats, pathset& images, pathset& failed);

int main (int argc, char const* argv[])
{
	// Check if we've got imagemagick
	if (::system("convert --version &> /dev/null") != 0) {
		cerr << "Requires the \"convert\" program from ImageMagick" << endl;
		return 1;
	}
	
	Stats stats;
	// See if there are paths on the arg list or we should wait for stdin
	// Put args into a vector
	strings args;
	for (int i=1; i<argc; i++) args.push_back(argv[i]);
	
	// Look for flags
	for (strings::iterator it=args.begin(); it!=args.end(); it++) {
		if ((*it).compare("-r") == 0) {
			stats.recurse = true;
			it = args.erase(it);
		}
	}
	
	// See if we should use stdin instead
	if (args.empty()) {
		string line;
		while (!cin.eof()) {
			if (!cin.good()) {
				cerr << "Error reading stdin" << endl;
				return 1;
			}
			// Read a line
			getline(cin, line);
			if (!line.empty()) args.push_back(line);
		}
	}
	args.sort();
	args.unique();
	if (args.empty()) {
		cerr << "No input paths!" << endl;
		return 1;
	}
	
	// Recurse folders synchronously to refresh html pages, delete stray hiddens, and build a list of files
	pathset images, thumbs;
	for (strings::iterator it=args.begin(); it!=args.end(); it++) {
		path p(*it);
		if (is_directory(p)) {
			stats.paths++;
			doFolder(stats, p, images);
			if (stats.recurse) {
				// Recurse
				for (boost::filesystem::recursive_directory_iterator end, it(p); it!=end; it++) {
					if (is_directory(*it)) {
						stats.paths++;
						doFolder(stats, *it, images);
					}
				}
			}
		} else {
			cerr << "Not a folder: " << p << endl;
		}
	}
	
	// Process the list of files and thumbs
	pathset failed;
	process(stats, images, failed);
	
	// Send the failed image names to a file
	if (!failed.empty()) {
		cerr << "Writing " << failed.size() << " failed image names to ./failed" << endl;
		ofstream failed_file("failed");
		for (pathset::iterator it=failed.begin(); it!=failed.end(); it++) {
			if (failed_file.good()) {
				failed_file << (*it).string() << endl;
			} else {
				cerr << "Problem writing to ./failed" << endl;
				break;
			}
		}
		failed_file.close();
	} else {
		cerr << "Success!" << endl;
	}
	
	// Report
	cout << "[images]  processed(" << stats.images_processed << ")"
	     << "  failed(" << stats.images_failed << ")"
	     << "  skipped(" << stats.images_skipped << ")"
	     << "  total(" << (stats.images_processed + stats.images_failed + stats.images_skipped) << ")"
	     << endl
	     << "[paths]   input(" << stats.paths << ")"
	     << endl;
	
	
	return 0;
}













