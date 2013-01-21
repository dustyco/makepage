

# Add source file names to groups
add_sources(makepage makepage.cpp doFolder.cpp process.cpp)

# Add library names to groups
require_libraries(makepage
	${Boost_THREAD_LIBRARY}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
	/usr/lib/libexiv2.a
	/usr/lib/libexpat.a
	/usr/lib/libz.a
	)
if(UNIX)
	require_libraries(makepage pthread)
endif()

# Call last to pass group variables up to the parent
propagate_groups()
