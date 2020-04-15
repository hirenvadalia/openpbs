add_library(site STATIC
	site_alt_rte.c
	site_mom_chu.c
	site_mom_jst.c
	site_allow_u.c
	site_check_u.c
	site_map_usr.c
	site_mom_ckp.c
)
target_include_directories(site PRIVATE
	${CMAKE_SOURCE_DIR}/src/include
	${CMAKE_SOURCE_DIR}/src/resmom/${PBS_MACH}
	${CMAKE_BINARY_DIR}/src/include)

add_library(pbs::lib::site ALIAS site)
