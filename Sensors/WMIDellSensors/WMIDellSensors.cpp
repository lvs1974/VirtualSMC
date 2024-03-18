//
//  WMIDellSensors.cpp
//  WMIDellSensors
//
//  Copyright © 2020 lvs1974. All rights reserved.
//

#include <VirtualSMCSDK/kern_vsmcapi.hpp>

#include "WMIDellSensors.hpp"
#include "KeyImplementations.hpp"
#include <Headers/plugin_start.hpp>
#include <Headers/kern_version.hpp>
#include <IOKit/pwr_mgt/RootDomain.h>

#include "kern_hooks.hpp"

OSDefineMetaClassAndStructors(WMIDellSensors, IOService)

bool WMIDellSensors::init(OSDictionary *properties) {
	if (!IOService::init(properties)) {
		return false;
	}

	if (!SMIMonitor::getShared())
		SMIMonitor::createShared();

	SMIMonitor::getShared()->fanMult = 1; //linux proposed to get nominal speed and if it high then change multiplier
	OSNumber * Multiplier = OSDynamicCast(OSNumber, properties->getObject("FanMultiplier"));
	if (Multiplier)
		SMIMonitor::getShared()->fanMult = Multiplier->unsigned32BitValue();
	return true;
}

IOService *WMIDellSensors::probe(IOService *provider, SInt32 *score) {
	auto ptr = IOService::probe(provider, score);
	if (!ptr) {
		SYSLOG("sdell", "failed to probe the parent");
		return nullptr;
	}

	wmiDevice = WMIDellDevice::probe();
	if (wmiDevice == nullptr) {
		SYSLOG("sdell", "WMIDellDevice could not be created.");
		return nullptr;
	}
	
	if (!SMIMonitor::getShared()->probe())
		return nullptr;
	
	tryToFindRsubDevice();

	auto fanCount = min(SMIMonitor::getShared()->fanCount, MaxIndexCount);
	VirtualSMCAPI::addKey(KeyFNum, vsmcPlugin.data,
		VirtualSMCAPI::valueWithUint8(fanCount, nullptr, SMC_KEY_ATTRIBUTE_CONST | SMC_KEY_ATTRIBUTE_READ));

	for (size_t i = 0; i < fanCount; i++) {
		VirtualSMCAPI::addKey(KeyF0Ac(i), vsmcPlugin.data, VirtualSMCAPI::valueWithFp(0, SmcKeyTypeFpe2, new F0Ac(i), SMC_KEY_ATTRIBUTE_WRITE | SMC_KEY_ATTRIBUTE_READ));
		VirtualSMCAPI::addKey(KeyF0Mn(i), vsmcPlugin.data, VirtualSMCAPI::valueWithFp(0, SmcKeyTypeFpe2, new F0Mn(i), SMC_KEY_ATTRIBUTE_WRITE | SMC_KEY_ATTRIBUTE_READ));
		VirtualSMCAPI::addKey(KeyF0Mx(i), vsmcPlugin.data, VirtualSMCAPI::valueWithFp(0, SmcKeyTypeFpe2, new F0Mx(i), SMC_KEY_ATTRIBUTE_WRITE | SMC_KEY_ATTRIBUTE_READ));
		VirtualSMCAPI::addKey(KeyF0Md(i), vsmcPlugin.data, VirtualSMCAPI::valueWithUint8(0, new F0Md(i), SMC_KEY_ATTRIBUTE_WRITE | SMC_KEY_ATTRIBUTE_READ));
		VirtualSMCAPI::addKey(KeyF0Tg(i), vsmcPlugin.data, VirtualSMCAPI::valueWithFp(0, SmcKeyTypeFpe2, new F0Tg(i), SMC_KEY_ATTRIBUTE_WRITE | SMC_KEY_ATTRIBUTE_READ));
	}
	VirtualSMCAPI::addKey(KeyFS__, vsmcPlugin.data,
		VirtualSMCAPI::valueWithUint16(0, new FS__(), SMC_KEY_ATTRIBUTE_WRITE | SMC_KEY_ATTRIBUTE_READ));

	OSArray* fanNames = OSDynamicCast(OSArray, getProperty("FanNames"));
	OSArray* stopFanOffsets = OSDynamicCast(OSArray, getProperty("StopFanOffsets"));
	char fan_name[DiagFunctionStrLen];

	for (unsigned int i = 0, cpu = 0; i < fanCount; i++) {
		FanTypeDescStruct	desc;
		FanInfo::FanType type = SMIMonitor::getShared()->state.fanInfo[i].type;
		if (type == FanInfo::Unsupported) {
			auto auto_type = (cpu++ == 0) ? FanInfo::CPU : FanInfo::GPU;
			DBGLOG("sdell", "Fan type %d is unknown, auto assign value %d", type, auto_type);
			type = auto_type;
		}
		snprintf(fan_name, DiagFunctionStrLen, "Fan %u", i);
		if (fanNames && type < fanNames->getCount()) {
			OSString* name = OSDynamicCast(OSString, fanNames->getObject(type));
			if (name)
				lilu_os_strncpy(fan_name, name->getCStringNoCopy(), DiagFunctionStrLen);
		}
		if (stopFanOffsets && i < stopFanOffsets->getCount()) {
			OSNumber* offset = OSDynamicCast(OSNumber, stopFanOffsets->getObject(i));
			if (offset)
				SMIMonitor::getShared()->state.fanInfo[i].stopOffset = offset->unsigned32BitValue();
		}
		lilu_os_strncpy(desc.strFunction, fan_name, DiagFunctionStrLen);
		VirtualSMCAPI::addKey(KeyF0ID(i), vsmcPlugin.data, VirtualSMCAPI::valueWithData(
			reinterpret_cast<const SMC_DATA *>(&desc), sizeof(desc), SmcKeyTypeFds, nullptr, SMC_KEY_ATTRIBUTE_CONST | SMC_KEY_ATTRIBUTE_READ));
	}

	auto tempCount = min(SMIMonitor::getShared()->tempCount, MaxIndexCount);
	for (size_t i = 0; i < tempCount; i++) {
		TempInfo::SMMTempSensorType type = SMIMonitor::getShared()->state.tempInfo[i].type;
		if (type <= TempInfo::Unsupported || type >= TempInfo::Last) {
			DBGLOG("sdell", "Temp sensor type %d is unknown, auto assign value %d", type, SMIMonitor::getShared()->state.tempInfo[i].index);
			type = static_cast<TempInfo::SMMTempSensorType>(SMIMonitor::getShared()->state.tempInfo[i].index);
			SMIMonitor::getShared()->state.tempInfo[i].type = type;
		}

		switch (type)
		{
		case TempInfo::CPU:
			DBGLOG("sdell", "CPU Proximity sensor is handled by SMCProcessor plugin");
			break;
		case TempInfo::GPU:
			VirtualSMCAPI::addKey(KeyTG0P(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TG0P(i),
				SMC_KEY_ATTRIBUTE_WRITE|SMC_KEY_ATTRIBUTE_READ));
			break;
		case TempInfo::Memory:
			VirtualSMCAPI::addKey(KeyTGVP, vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TGVP(i),
				SMC_KEY_ATTRIBUTE_WRITE|SMC_KEY_ATTRIBUTE_READ));
			break;
		case TempInfo::Misc:
			VirtualSMCAPI::addKey(KeyTN0P(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TN0P(i),
				SMC_KEY_ATTRIBUTE_WRITE|SMC_KEY_ATTRIBUTE_READ));
			break;
		case TempInfo::Ambient:
			VirtualSMCAPI::addKey(KeyTA0P(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TA0P(i),
				SMC_KEY_ATTRIBUTE_WRITE|SMC_KEY_ATTRIBUTE_READ));
			break;
		case TempInfo::Other:
			VirtualSMCAPI::addKey(KeyTW0P(0), vsmcPlugin.data, VirtualSMCAPI::valueWithSp(0, SmcKeyTypeSp78, new TW0P(i),
				SMC_KEY_ATTRIBUTE_WRITE|SMC_KEY_ATTRIBUTE_READ));
			break;
		default:
			DBGLOG("sdell", "Temp sensor type %d is unsupported", type);
			break;
		}
	}

	qsort(const_cast<VirtualSMCKeyValue *>(vsmcPlugin.data.data()), vsmcPlugin.data.size(), sizeof(VirtualSMCKeyValue), VirtualSMCKeyValue::compare);

	return this;
}

void WMIDellSensors::tryToFindRsubDevice()
{
	auto dict = IOService::nameMatching("AppleACPIPlatformExpert");
	if (!dict) {
		SYSLOG("sdell", "WTF? Failed to create matching dictionary");
		return;
	}
	
	auto acpi = IOService::waitForMatchingService(dict);
	dict->release();
	
	if (!acpi) {
		SYSLOG("sdell", "WTF? No ACPI");
		return;
	}
	
	acpi->release();
	
	dict = IOService::nameMatching("RSUB001");
	if (!dict) {
		SYSLOG("sdell", "WTF? Failed to create matching dictionary");
		return;
	}
	
	auto deviceIterator = IOService::getMatchingServices(dict);
	dict->release();
	
	if (!deviceIterator) {
		SYSLOG("sdell", "No iterator");
		return;
	}

	rsubDevice = OSDynamicCast(IOACPIPlatformDevice, deviceIterator->getNextObject());
	deviceIterator->release();
	
	if (!rsubDevice) {
		SYSLOG("sdell", "RSUB001 device not found");
		return;
	}

	if (rsubDevice->validateObject("HOTP") != kIOReturnSuccess) {
		SYSLOG("sdell", "No functional HOTP method on RSUB device");
		return;
	}
	
	DBGLOG("sdell", "RSUB device has been found");
}

bool WMIDellSensors::start(IOService *provider) {
	if (!IOService::start(provider)) {
		SYSLOG("sdell", "failed to start the parent");
		return false;
	}

	setProperty("VersionInfo", kextVersion);

	notifier = registerSleepWakeInterest(IOSleepHandler, this);
	if (notifier == nullptr) {
		SYSLOG("sdell", "failed to register sleep/wake interest");
		return false;
	}
	
	if (!eventTimer) {
		if (!workLoop)
			workLoop = IOWorkLoop::workLoop();

		if (workLoop) {
			eventTimer = IOTimerEventSource::timerEventSource(workLoop,
			[](OSObject *owner, IOTimerEventSource *) {
				SMIMonitor::getShared()->handlePowerOn();
			});

			if (eventTimer) {
				IOReturn result = workLoop->addEventSource(eventTimer);
				if (result != kIOReturnSuccess) {
					SYSLOG("sdell", "WMIDellSensors addEventSource failed");
					OSSafeReleaseNULL(eventTimer);
				}
			}
			else
				SYSLOG("sdell", "WMIDellSensors timerEventSource failed");
		}
		else
			SYSLOG("sdell", "WMIDellSensors IOService instance does not have workLoop");
	}

	SMIMonitor::getShared()->start();

	PMinit();
	provider->joinPMtree(this);
	registerPowerDriver(this, powerStates, arrsize(powerStates));
	registerService();

	vsmcNotifier = VirtualSMCAPI::registerHandler(vsmcNotificationHandler, this);
	return vsmcNotifier != nullptr;
}

bool WMIDellSensors::vsmcNotificationHandler(void *sensors, void *refCon, IOService *vsmc, IONotifier *notifier) {
	if (sensors && vsmc) {
		DBGLOG("sdell", "got vsmc notification");
		auto &plugin = static_cast<WMIDellSensors *>(sensors)->vsmcPlugin;
		auto ret = vsmc->callPlatformFunction(VirtualSMCAPI::SubmitPlugin, true, sensors, &plugin, nullptr, nullptr);
		if (ret == kIOReturnSuccess) {
			DBGLOG("sdell", "submitted plugin");
			return true;
		} else if (ret != kIOReturnUnsupported) {
			SYSLOG("sdell", "plugin submission failure %X", ret);
		} else {
			DBGLOG("sdell", "plugin submission to non vsmc");
		}
	} else {
		SYSLOG("sdell", "got null vsmc notification");
	}
	return false;
}

void WMIDellSensors::stop(IOService *provider) {
	PANIC("sdell", "called stop!!!");
}

IOReturn WMIDellSensors::setPowerState(unsigned long state, IOService *whatDevice){
	DBGLOG("sdell", "changing power state to %lu", state);
	return kIOPMAckImplied;
}

IOReturn WMIDellSensors::IOSleepHandler(void *target, void *, UInt32 messageType, IOService *provider, void *messageArgument, vm_size_t)
{
	sleepWakeNote *swNote = (sleepWakeNote *)messageArgument;

	if (messageType != kIOMessageSystemWillSleep &&
		messageType != kIOMessageSystemHasPoweredOn &&
		messageType != kIOMessageSystemWillPowerOff &&
		messageType != kIOMessageSystemWillRestart) {
		return kIOReturnUnsupported;
	}

	DBGLOG("sdell", "IOSleepHandler message type = 0x%x", messageType);

	swNote->returnValue = 0;
	if (messageType != kIOMessageSystemHasPoweredOn)
		acknowledgeSleepWakeNotification(swNote->powerRef);

	auto that = OSDynamicCast(WMIDellSensors, reinterpret_cast<WMIDellSensors*>(target));

	if (messageType == kIOMessageSystemWillSleep || messageType == kIOMessageSystemWillPowerOff || messageType == kIOMessageSystemWillRestart) {
		if (that && that->eventTimer)
			that->eventTimer->cancelTimeout();
		SMIMonitor::getShared()->handlePowerOff();
	} else if (messageType == kIOMessageSystemHasPoweredOn) {
		auto that = OSDynamicCast(WMIDellSensors, reinterpret_cast<WMIDellSensors*>(target));
		if (that && that->eventTimer)
			that->eventTimer->setTimeoutMS(20000);
		else
			SMIMonitor::getShared()->handlePowerOn();
	}

	return kIOReturnSuccess;
}

bool WMIDellSensors::evaluate(WMI_CLASS smi_class, WMI_SELECTOR select, const int_array args, int_array &res)
{
	switch (smi_class)
	{
		case WMI_CLASS::RsubCall: {
			if (rsubDevice == nullptr)
				return kIOReturnUnsupported;
			auto result = rsubDevice->evaluateObject("HOTP");
			DBGLOG("sdell", "WMIDellSensors: evaluateObject(HOTP) result = %x", result);
			return result;
		}
	
		case WMI_CLASS::RefreshSensors: {
			SMIMonitor::getShared()->postSmcUpdate(KeyRFSH, 0, 0, 0, true);
			return KERN_SUCCESS;
		}
	
		case WMI_CLASS::AutoFanMode: {
			// turn off manual control for all fans
			UInt16 data = 0;
			SMIMonitor::getShared()->postSmcUpdate(KeyFS__, -1, &data, sizeof(data), true);
			return KERN_SUCCESS;
		}
	
		case WMI_CLASS::ManualFanMode: {
			// turn on manual control for all fans
			UInt16 data = 0xffff;
			SMIMonitor::getShared()->postSmcUpdate(KeyFS__, -1, &data, sizeof(data), true);
			return KERN_SUCCESS;
		}
	
		case WMI_CLASS::LeftOff: {
			UInt8 data = 0;
			SMIMonitor::getShared()->postSmcUpdate(KeyFaMD, 0, &data, sizeof(data), true);
			return KERN_SUCCESS;
		}

		case WMI_CLASS::LeftMedium: {
			UInt8 data = 1;
			SMIMonitor::getShared()->postSmcUpdate(KeyFaMD, 0, &data, sizeof(data), true);
			return KERN_SUCCESS;
		}

		case WMI_CLASS::LeftHigh: {
			UInt8 data = 2;
			SMIMonitor::getShared()->postSmcUpdate(KeyFaMD, 0, &data, sizeof(data), true);
			return KERN_SUCCESS;
		}
	
		case WMI_CLASS::RightOff: {
			UInt8 data = 0;
			SMIMonitor::getShared()->postSmcUpdate(KeyFaMD, 1, &data, sizeof(data), true);
			return KERN_SUCCESS;
		}

		case WMI_CLASS::RightMedium: {
			UInt8 data = 1;
			SMIMonitor::getShared()->postSmcUpdate(KeyFaMD, 1, &data, sizeof(data), true);
			return KERN_SUCCESS;
		}

		case WMI_CLASS::RightHigh: {
			UInt8 data = 2;
			SMIMonitor::getShared()->postSmcUpdate(KeyFaMD, 1, &data, sizeof(data), true);
			return KERN_SUCCESS;
		}
	
	default:
		break;
	}

	return wmiDevice->evaluate(smi_class, select, args, res);
}

IOReturn WMIDellSensors::runAction(UInt32 action, UInt32 *outSize, void **outData, void *extraArg)
{
	return kIOReturnSuccess;
}

IOReturn WMIDellSensors::newUserClient(task_t owningTask, void * securityID, UInt32 type, IOUserClient ** handler )
{
	IOReturn ioReturn = kIOReturnSuccess;
	WMIDellUserClient *client = nullptr;

	if (mClientCount > MAXUSERS)
	{
		SYSLOG("sdell", "WMIDellSensors: Client already created, not delete");
		return(kIOReturnError);
	}
	
	client = (WMIDellUserClient *)WMIDellUserClient::withTask(owningTask);
	if (client == nullptr) {
		ioReturn = kIOReturnNoResources;
		SYSLOG("sdell", "WMIDellSensors::newUserClient: Can't create user client");
	}
	
	if (ioReturn == kIOReturnSuccess) {
		// Start the client so it can accept requests.
		client->attach(this);
		if (client->start(this) == false) {
			ioReturn = kIOReturnError;
			SYSLOG("sdell", "VoltageShiftAnVMSR::newUserClient: Can't start user client");
		}
	}
	
	if (ioReturn != kIOReturnSuccess && client != nullptr) {
		IOLog("WMIDellSensors: newUserClient error");
		client->detach(this);
		client->release();
	} else {
		mClientPtr[mClientCount] = client;
		
		*handler = client;
		
		client->set_Q_Size(type);
		mClientCount++;
	}
	
	DBGLOG("sdell", "WMIDellSensors: newUserClient() client = %p", mClientPtr[mClientCount]);
	return (ioReturn);
}

void WMIDellSensors::closeChild(WMIDellUserClient *ptr)
{
	UInt8 i, idx;
	idx = 0;
	
	if (mClientCount == 0)
	{
		SYSLOG("sdell", "WMIDellSensors: No clients available to close");
		return;
	}
	
	DBGLOG("sdell", "WMIDellSensors: Closing: %p",ptr);
	for(i=0; i<mClientCount; i++)
	{
		DBGLOG("sdell", "WMIDellSensors:userclient ref: %d %p", i, mClientPtr[i]);
	}
	
	for(i=0; i<mClientCount; i++)
	{
		if (mClientPtr[i] == ptr)
		{
			mClientCount--;
			mClientPtr[i] = nullptr;
			idx = i;
			i = mClientCount+1;
		}
	}
	
	for(i=idx; i<mClientCount; i++)
	{
		mClientPtr[i] = mClientPtr[i+1];
	}
	mClientPtr[mClientCount+1] = nullptr;
}


OSDefineMetaClassAndStructors(WMIDellUserClient, IOUserClient);

const WMIDellUserClient *WMIDellUserClient::withTask(task_t owningTask)
{
	WMIDellUserClient *client;
	
	client = new WMIDellUserClient;
	if (client != nullptr)
	{
		if (client->init() == false)
		{
			client->release();
			client = nullptr;
		}
	}
	if (client != nullptr)
		client->fTask = owningTask;
	return (client);
}

bool WMIDellUserClient::set_Q_Size(UInt32 capacity)
{
	if (capacity == 0)
		return true;

	DBGLOG("sdell", "WMIDellUserClient:  Reseting size of data queue, all data in queue is lost");

	//Get mem for new queue of calcuated size
	return true;
}

void WMIDellUserClient::free()
{
	DBGLOG("sdell", "WMIDellUserClient:  AnVMSRUserClient::free");
	mDevice->release();
	IOUserClient::free();
}

bool WMIDellUserClient::start(IOService *provider)
{
	DBGLOG("sdell", "WMIDellUserClient: starting with provider %s...", safeString(provider->getName()));
	
	if (!IOUserClient::start(provider))
		return false;
	
	mDevice = OSDynamicCast(WMIDellSensors, provider);
	mDevice->retain();
	
	return true;
}

void WMIDellUserClient::stop(IOService *provider)
{
	DBGLOG("sdell", "WMIDellUserClient: stop with provider %s...", safeString(provider->getName()));
	IOUserClient::stop(provider);
}

bool WMIDellUserClient::initWithTask(task_t owningTask, void *securityID, UInt32 type, OSDictionary *properties)
{
	return IOUserClient::initWithTask(owningTask, securityID, type, properties);
}

// clientClose is called when the user process calls IOServiceClose
IOReturn WMIDellUserClient::clientClose()
{
	if (mDevice != nullptr)
		mDevice->closeChild(this);

	if (!isInactive())
		terminate();
	
	return kIOReturnSuccess;
}

// clientDied is called when the user process terminates unexpectedly, the default
// implementation simply calls clientClose
IOReturn WMIDellUserClient::clientDied()
{
	return clientClose();
}

bool WMIDellUserClient::willTerminate(IOService *provider, IOOptionBits options)
{
	DBGLOG("sdell", "WMIDellUserClient: willTerminate with provider %s...", safeString(provider->getName()));
	return IOUserClient::willTerminate(provider, options);
}

bool WMIDellUserClient::didTerminate(IOService *provider, IOOptionBits options, bool *defer)
{
	DBGLOG("sdell", "WMIDellUserClient: didTerminate with provider %s...", safeString(provider->getName()));
	
	// if defer is true, stop will not be called on the user client
	*defer = false;
	
	return IOUserClient::didTerminate(provider, options, defer);
}

bool WMIDellUserClient::terminate(IOOptionBits options)
{
	return IOUserClient::terminate(options);
}

// getTargetAndMethodForIndex looks up the external methods - supply a description of the parameters
// available to be called
IOExternalMethod *WMIDellUserClient::getTargetAndMethodForIndex(IOService **target, UInt32 index)
{
	static IOExternalMethod methodDescs[1] = {
		{ nullptr, (IOMethod) &WMIDellUserClient::actionEvaluate, kIOUCStructIStructO,
			kIOUCVariableStructureSize, kIOUCVariableStructureSize }
	};
	
	*target = this;
	if (index == 0)
		return reinterpret_cast<IOExternalMethod *>(methodDescs);

	return nullptr;
}

IOReturn WMIDellUserClient::clientMemoryForType(UInt32 type, IOOptionBits *options, IOMemoryDescriptor **memory)
{
	IOBufferMemoryDescriptor *memDesc;
	char *msgBuffer;

	*options = 0;
	*memory = nullptr;
	
	memDesc = IOBufferMemoryDescriptor::withOptions(kIOMemoryKernelUserShared, mDevice->mPrefPanelMemoryBufSize);

	if (!memDesc)
	{
		DBGLOG("sdell", "WMIDellUserClient: mempory could not be allocated in clientMemoryForType");
		return kIOReturnUnsupported;
	}

	msgBuffer = (char *) memDesc->getBytesNoCopy();
	bcopy(&mDevice->mPrefPanelMemoryBuf, msgBuffer, mDevice->mPrefPanelMemoryBufSize);
	*memory = memDesc; // automatically released after memory is mapped into task

	return kIOReturnSuccess;
}

IOReturn WMIDellUserClient::actionEvaluate(UInt32 *dataIn, UInt32 *dataOut, IOByteCount inputSize, IOByteCount *outputSize)
{
	calling_interface_buffer *indata  = reinterpret_cast<calling_interface_buffer*>(dataIn);
	calling_interface_buffer *outdata = reinterpret_cast<calling_interface_buffer*>(dataOut);

	DBGLOG("sdell", "WMIDellUserClient: actionEvaluate called");
	
	if (!dataIn) {
		SYSLOG("sdell", "WMIDellUserClient:actionEvaluate called without dataIn");
		return kIOReturnUnsupported;
	}

	if (!dataOut)
	{
		SYSLOG("sdell", "WMIDellUserClient:actionEvaluate called without dataOut");
		return kIOReturnUnsupported;
	}

	if (!mDevice->evaluate(indata->cmd_class, indata->cmd_select, indata->input, outdata->output))
		outdata->output[3] = kIOReturnUnsupported;

	return kIOReturnSuccess;
}
