set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
#set(CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE ON)
set(CPACK_PACKAGE_VENDOR "Martin Gerhardy")
set(CPACK_PACKAGE_CONTACT "martin.gerhardy@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION "Voxel engine and tools")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Voxel engine")
#set(CPACK_RESOURCE_FILE_LICENSE ${ROOT_DIR}/LICENSE)
set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
set(CPACK_SOURCE_IGNORE_FILES "~$")

if (UNIX)
	set(CPACK_GENERATOR "TGZ;TBZ2")
	find_host_program(RPMBUILD rpmbuild)
	if (RPMBUILD)
		list(APPEND CPACK_GENERATOR RPM)
		set(CPACK_RPM_COMPONENT_INSTALL ON)
		set(CPACK_RPM_PACKAGE_LICENSE GPL)
		set(CPACK_RPM_PACKAGE_REQUIRES)
	endif()
	find_host_program(DPKG dpkg)
	if (DPKG)
		find_program(SHLIBDEPS_EXECUTABLE dpkg-shlibdeps)
		if (NOT SHLIBDEPS_EXECUTABLE)
			message(WARNING "No dpkg-shlibdeps found")
		endif()
		list(APPEND CPACK_GENERATOR DEB)
		set(CPACK_DEBIAN_ENABLE_COMPONENT_DEPENDS ON)
		set(CPACK_DEB_COMPONENT_INSTALL ON)
		set(CPACK_DEB_PACKAGE_COMPONENT ON)
		set(CPACK_DEBIAN_PACKAGE_DEBUG ON)
		set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
		set(CPACK_DEBIAN_PACKAGE_SECTION games)
		set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${CPACK_PACKAGE_VENDOR})
	endif()
	find_host_program(NSIS nsisbuild)
	if (NSIS)
		list(APPEND CPACK_GENERATOR NSIS)
		set(CPACK_NSIS_COMPRESSOR bzip2)
		set(CPACK_NSIS_URL_INFO_ABOUT ${CPACK_PACKAGE_CONTACT})
	endif()
	set(CPACK_SOURCE_GENERATOR "TGZ")
elseif (WINDOWS)
	set(CPACK_GENERATOR "NSIS")
	set(CPACK_SOURCE_GENERATOR "ZIP")
	set(CPACK_NSIS_COMPRESSOR bzip2)
	set(CPACK_NSIS_URL_INFO_ABOUT ${CPACK_PACKAGE_CONTACT})
endif()
