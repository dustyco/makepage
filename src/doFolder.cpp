
#include <utility>
#include <set>
#include <list>
#include <vector>
#include <fstream>
#include <iostream>
using namespace std;

#include <boost/filesystem.hpp>
using namespace boost::filesystem;
#include <boost/algorithm/string.hpp>
#include <boost/assign/list_of.hpp>
using namespace boost;

#include "Stats.h"

typedef set<path> pathset;
typedef list<path> pathlist;

#include "CONFIGURE.h"


void doHeader (ostream& html, const path& p) {
	html << "<html>" << endl
	     << "<head>" << endl
	     << "<title>" << p.filename().string() << "</title>" << endl
	     << "<meta charset=\"UTF-8\" />" << endl
	     << "</head>" << endl
		 << "<style>" << endl
		 << "	body {" << endl
		 << "		font-family:monospace;" << endl
		 << "		color:#b0b0b0;" << endl
		 << "		background-color:#222222;" << endl
		 << "		font-size:16;" << endl
		 << "		margin:0;" << endl
		 << "		padding:0;" << endl
		 << "	}" << endl
		 << "	a {" << endl
		 << "		color:#ffffff;" << endl
		 << "		text-decoration:none;" << endl
		 << "		background-color:#333333;" << endl
		 << "	}" << endl
		 << "	a.folder {" << endl
		 << "		padding:4px;" << endl
		 << "		color:#5f9fcf;" << endl
		 << "	}" << endl
		 << "	a.file {" << endl
		 << "		padding:4px;" << endl
		 << "	}" << endl
		 << "	td {" << endl
		 << "		vertical-align:top;" << endl
		 << "	}" << endl
		 << "	img.fullscreen {" << endl
		 << "		height:100%;" << endl
		 << "		width:auto;" << endl
		 << "	}" << endl
		 << "</style>" << endl
		 << "<body>" << endl
		 << "<div id=\"imgdiv\"></div>" << endl
#ifdef COLUMNS
		 << "<div id=\"indexdiv\" style=\"display:none;\">" << endl
#endif
		 << endl << endl;

}

void doPathbar (ostream& html, path p) {
	html << "<h1>";
	pathlist nodes;
	
	if (p.filename().string() == ".") {
		p.remove_filename();
	}
	while (!p.empty()) {
		nodes.push_front(p.filename());
		p.remove_filename();
	}
	int depth = nodes.size();
	for (pathlist::iterator it=nodes.begin(); it!=nodes.end(); it++) {
		depth--;
		html << "<a class=\"folder\" href=\"";
		for (int i=0; i<depth; i++) {
			html << "../";
		}
		html << "index.html\">" << (*it).string() << "</a>/";
	}
	html << "<h1>" << endl;
}

void doMenu (ostream& html, pathset& files, pathset& folders) {
#ifdef COLUMNS
	html << "<div>" << endl
	     << "<table>" << endl;
#else
	html << "<div style=\"float:left;\">" << endl
	     << "<table>" << endl;
#endif
	for (pathset::iterator it=folders.begin(); it!=folders.end(); it++) {
		string folderstr = (*it).filename().string();
		html << "<tr><td><a class=\"folder\" href=\"" << folderstr << "/index.html\">" << folderstr << "/</a></td></tr>" << endl;
/*		html << "<tr><td><img src=\"" << folderstr << "/.icon.jpg\" /></td>"
		     << "<td> </td>"
//		     << "<td>"$(ls "$1" | grep -v "^$html$" | wc -l)"</td>"  // Item count
		     << "<td><a class=\"folder\" href=\"" << folderstr << "/index.html\">" << folderstr << "/</a></td></tr>" << endl;
*/
	}
	for (pathset::iterator it=files.begin(); it!=files.end(); it++) {
		string filestr = (*it).filename().string();
		html << "<tr><td><a class=\"file\" href=\"" << filestr << "\">" << filestr << "</a></td></tr>" << endl;
//		html << "<tr><td> </td><td> </td><td><a class=\"file\" href=\"" << filestr << "\">" << filestr << "</a></td></tr>" << endl;
	}
	html << "</table>" << endl
	     << "</div>" << endl << endl;
}

void doImages (ostream& html, pathset& images, pathlist& thumbs) {
#ifdef COLUMNS
	html << "<div id=\"container\">" << endl;
#endif
	for (pathset::iterator it=images.begin(); it!=images.end(); it++) {
		string imagename = (*it).filename().string();
		string thumbname = "." + imagename + ".jpg";
		path thumb = (*it);
		thumb.remove_filename();
		thumb += ("/" + thumbname);
		thumbs.push_back(thumb);
#ifdef COLUMNS
		html << "<div class=\"box photo col3\">";
#endif
		html << "<a target=\"_blank\" href=\"?file=" << imagename << "\"><img src=\"" << thumbname << "\" /></a>";
#ifdef COLUMNS
		html << "</div>" << endl;
#endif
	}
#ifdef COLUMNS
	html << "</div>";
#endif
	html << endl << endl;
}

void doFooter (ostream& html) {
#ifdef COLUMNS
	html << "</div>" << endl
	     << endl
	     << "<script>" << endl
	     << MASONRY << endl
	     << "</script>" << endl
#endif
	     << "<script>" << endl
	     << "function getUrlVars() {" << endl
	     << "	var map = {};" << endl
	     << "	var parts = window.location.href.replace(/[?&]+([^=&]+)=([^&]*)/gi, function(m,key,value) {" << endl
	     << "		map[key] = value;" << endl
	     << "	});" << endl
	     << "	return map;" << endl
	     << "};" << endl
	     << "function centerView() {" << endl
	     << "	scrollTo(document.body.scrollWidth/2 - document.body.clientWidth/2, 0);" << endl
	     << "}" << endl
	     << "window.onload = function() {" << endl
	     << "	var file = getUrlVars()[\"file\"];" << endl
	     << "	if (file) {" << endl
	     << "		document.getElementById('imgdiv').innerHTML='<img onload=\"centerView()\" class=\"fullscreen\" src=\"'+file+'\" />';" << endl
	     << "	} else {" << endl
	     << "		document.getElementById('indexdiv').style.display='block';" << endl
#ifdef COLUMNS
	     << "		var wall = new Masonry( document.getElementById('container') );" << endl
#endif
	     << "	}" << endl
	     << "};" << endl
	     << "</script>" << endl << endl
	     << "</body>" << endl
	     << "</html>" << endl;
}

static const vector<string> EXTENSIONS = assign::list_of(".jpg")(".jpeg")(".gif")(".png")(".bmp");

void doFolder (Stats& stats, const path& p, pathset& images_ret)
{
	// Populate lists with files and folders
	pathset files;
	pathset folders;
	pathset images;
	pathlist images_thumbs;
	pathset thumbs;
	pathset hidden;
	cout << p.string() << endl;
	directory_iterator end_itd;
	for (directory_iterator itd(p); itd!=end_itd; itd++) {
		path item(*itd);
		if (is_directory(*itd)) {
			folders.insert(item);
		} else if (is_regular_file(*itd)) {
			stats.images_skipped++;
			// Hidden?
			string filename = item.filename().string();
			if (filename[0] == '.') {
				hidden.insert(item);
//				cout << "\tHidden: " << item.filename() << endl;
				continue;
			}
			if (filename == "index.html") continue;
			// Check if it's an image
			string ext = item.extension().string(); to_lower(ext);
			for (vector<string>::const_iterator EXT=EXTENSIONS.begin();; EXT++) {
				if (EXT==EXTENSIONS.end()) {
					files.insert(item);
//					cout << "\tFile: " << item.filename() << endl;
					break;
				} else if (ext == *EXT) {
					images.insert(item);
//					cout << "\tImage: " << item.filename() << endl;
					break;
				}
			}
		}
	}
	// Write the html
	ofstream html((p.string() + "/index.html").c_str());
	doHeader(html, p);
	doPathbar(html, p);
	doMenu(html, files, folders);
	doImages(html, images, images_thumbs);
	doFooter(html);
	html.close();
	
	// See what thumbs need to be deleted
	for (pathlist::iterator it=images_thumbs.begin(); it!=images_thumbs.end(); it++) {
		// Check if the thumb is up to date
		
		// Remove from delete list if the thumb is ok
		hidden.erase(*it);
	}
	// Delete stray or old thumbs
	for (pathset::iterator it=hidden.begin(); it!=hidden.end(); it++) {
		cerr << "\t[Deleting]  " << *it << endl;
		remove(*it);
	}
	
	// Return image and thumb paths
	images_ret.insert(images.begin(), images.end());
	
//	makeThumbs(images, images_thumbs);
}




