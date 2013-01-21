

struct Stats {
	long paths;
	long images_processed;
	long images_failed;
	long images_skipped;
	bool recurse;
	Stats () :
		paths(0),
		images_processed(0),
		images_failed(0),
		images_skipped(0),
		recurse(false)
		{}
};
