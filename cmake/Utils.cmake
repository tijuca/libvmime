
# Installing headers and preserving the directory structure
# Found here: http://www.semipol.de/archives/251
MACRO(INSTALL_HEADERS_WITH_DIRECTORY HEADER_LIST COMPONENT_NAME REMOVE_PREFIX)

	FOREACH(HEADER ${${HEADER_LIST}})
		STRING(REGEX MATCH "(.*)[/\\]" DIR ${HEADER})
		STRING(REPLACE "${REMOVE_PREFIX}" "" DIR ${DIR})
		INSTALL(FILES ${HEADER} DESTINATION include/${DIR} COMPONENT ${COMPONENT_NAME})
	ENDFOREACH(HEADER)

ENDMACRO(INSTALL_HEADERS_WITH_DIRECTORY)

