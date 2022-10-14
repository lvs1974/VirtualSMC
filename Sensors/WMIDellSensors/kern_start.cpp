//
//  kern_start.cpp
//  WMIDellSensors
//
//  Copyright © 2020 lvs1974. All rights reserved.
//

#include <Headers/plugin_start.hpp>
#include <Headers/kern_api.hpp>

static const char *bootargOff[] {
	"-delloff"
};

static const char *bootargDebug[] {
	"-delldbg"
};

static const char *bootargBeta[] {
	"-dellbeta"
};

PluginConfiguration ADDPR(config) {
	xStringify(PRODUCT_NAME),
	parseModuleVersion(xStringify(MODULE_VERSION)),
	LiluAPI::AllowNormal | LiluAPI::AllowInstallerRecovery | LiluAPI::AllowSafeMode,
	bootargOff,
	arrsize(bootargOff),
	bootargDebug,
	arrsize(bootargDebug),
	bootargBeta,
	arrsize(bootargBeta),
	KernelVersion::MountainLion,
	KernelVersion::Monterey,
	[]() {
	}
};
