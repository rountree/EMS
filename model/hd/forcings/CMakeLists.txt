target_sources(shoc
	PRIVATE
	airtemp.c
	cloud.c
	crash.c
	dhw.c
	eta_relax.c
	evap.c
	forcings.c
	heatflux.c
	hftemp.c
	patm.c
	precip.c
	regulate.c
	rh.c
	storm.c
	swr.c
	vel_relax.c
	webf.c
	wetbulb.c
	wind.c
)

# light.c present in directory but not part of build process
