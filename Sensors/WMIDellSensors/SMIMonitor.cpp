/*
 *  SMIMonitor.cpp
 *  WMIDellSensors
 *
 *  Copyright 2014 Slice. All rights reserved.
 *  Adapted for VirtualSMC by lvs1974, 2020
 *  Original sources: https://github.com/CloverHackyColor/FakeSMC3_with_plugins
 *
 */

#include "SMIMonitor.hpp"

#include <Headers/kern_cpu.hpp>
#include <Headers/kern_util.hpp>
#include "kern_hooks.hpp"

extern "C" {
#include <i386/pmCPU.h>
}

//extern "C" int dell_smm_lowlevel(SMBIOS_PKG *smm_pkg);

SMIMonitor *SMIMonitor::instance = nullptr;
atomic_bool SMIMonitor::busy = 0;

OSDefineMetaClassAndStructors(SMIMonitor, OSObject)


int SMIMonitor::i8k_smm(SMBIOS_PKG *sc, bool force_access) {
	
//	int result = dell_smm_lowlevel(sc);
//	return result;
	
	int attempts = 50;
	while (atomic_load_explicit(&KERNELHOOKS::active_output, memory_order_acquire) && --attempts >= 0) { IOSleep(10); }
	if (attempts < 0 && !force_access)
		return -1;
	
	atomic_store_explicit(&busy, true, memory_order_release);
	
	if (atomic_load_explicit(&KERNELHOOKS::active_output, memory_order_acquire) && !force_access) {
		DBGLOG("sdell", "stop accessing smm, active_outputs = %d", atomic_load_explicit(&KERNELHOOKS::active_output, memory_order_acquire));
		atomic_store_explicit(&busy, false, memory_order_release);
		return -1;
	}
	
	SMMRegisters r{sc->cmd, sc->data, sc->stat1, sc->stat2};
	SMMRegisters *regs = &r;

	int rc;
	int eax = regs->eax;  //input value
	
#if __LP64__
	asm volatile("pushq %%rax\n\t"
			"movl 0(%%rax),%%edx\n\t"
			"pushq %%rdx\n\t"
			"movl 4(%%rax),%%ebx\n\t"
			"movl 8(%%rax),%%ecx\n\t"
			"movl 12(%%rax),%%edx\n\t"
			"movl 16(%%rax),%%esi\n\t"
			"movl 20(%%rax),%%edi\n\t"
			"popq %%rax\n\t"
			"out %%al,$0xb2\n\t"
			"out %%al,$0x84\n\t"
			"xchgq %%rax,(%%rsp)\n\t"
			"movl %%ebx,4(%%rax)\n\t"
			"movl %%ecx,8(%%rax)\n\t"
			"movl %%edx,12(%%rax)\n\t"
			"movl %%esi,16(%%rax)\n\t"
			"movl %%edi,20(%%rax)\n\t"
			"popq %%rdx\n\t"
			"movl %%edx,0(%%rax)\n\t"
			"pushfq\n\t"
			"popq %%rax\n\t"
			"andl $1,%%eax\n"
			: "=a"(rc)
			: "a"(regs)
			: "%ebx", "%ecx", "%edx", "%esi", "%edi", "memory");
#else
	asm volatile("pushl %%eax\n\t"
			"movl 0(%%eax),%%edx\n\t"
			"push %%edx\n\t"
			"movl 4(%%eax),%%ebx\n\t"
			"movl 8(%%eax),%%ecx\n\t"
			"movl 12(%%eax),%%edx\n\t"
			"movl 16(%%eax),%%esi\n\t"
			"movl 20(%%eax),%%edi\n\t"
			"popl %%eax\n\t"
			"out %%al,$0xb2\n\t"
			"out %%al,$0x84\n\t"
			"xchgl %%eax,(%%esp)\n\t"
			"movl %%ebx,4(%%eax)\n\t"
			"movl %%ecx,8(%%eax)\n\t"
			"movl %%edx,12(%%eax)\n\t"
			"movl %%esi,16(%%eax)\n\t"
			"movl %%edi,20(%%eax)\n\t"
			"popl %%edx\n\t"
			"movl %%edx,0(%%eax)\n\t"
			"lahf\n\t"
			"shrl $8,%%eax\n\t"
			"andl $1,%%eax\n"
			: "=a"(rc)
			: "a"(regs)
			: "%ebx", "%ecx", "%edx", "%esi", "%edi", "memory");
#endif
	
	atomic_store_explicit(&busy, false, memory_order_release);

	sc->cmd = regs->eax;
	sc->data = regs->ebx;
	sc->stat1 = regs->ecx;
	sc->stat2 = regs->edx;
	
	if ((rc != 0) || ((regs->eax & 0xffff) == 0xffff) || (regs->eax == eax)) {
		return -1;
	}

	return 0;
}


/*
 * Read the CPU temperature in Celcius.
 */
int SMIMonitor::i8k_get_temp(int sensor, bool force_access) {
	SMBIOS_PKG smm_pkg {};
	int rc;
	int temp;

	smm_pkg.cmd = I8K_SMM_GET_TEMP;
	smm_pkg.data = sensor & 0xFF;
	if ((rc=i8k_smm(&smm_pkg, force_access)) < 0) {
		return rc;
	}

	temp = smm_pkg.cmd & 0xff;
	if (temp == 0x99) {
		IOSleep(100);
		smm_pkg.cmd = I8K_SMM_GET_TEMP;
		smm_pkg.data = sensor & 0xFF;
		if ((rc=i8k_smm(&smm_pkg, force_access)) < 0) {
			return rc;
		}
		temp = smm_pkg.cmd & 0xff;
	}
	return temp;
}

int SMIMonitor::i8k_get_temp_type(int sensor) {
	SMBIOS_PKG smm_pkg {};
	int rc;
	int type;

	smm_pkg.cmd = I8K_SMM_GET_TEMP_TYPE;
	smm_pkg.data = sensor & 0xFF;
	if ((rc=i8k_smm(&smm_pkg, true)) < 0) {
		return rc;
	}

	type = smm_pkg.cmd & 0xff;
	return type;
}

bool SMIMonitor::i8k_get_dell_sig_aux(int fn) {
	SMBIOS_PKG smm_pkg {};

	smm_pkg.cmd = fn;
	if (i8k_smm(&smm_pkg, true) < 0) {
		DBGLOG("sdell", "No function 0x%x", fn);
		return false;
	}
	DBGLOG("sdell", "Got sigs 0x%X and 0x%X", smm_pkg.cmd, smm_pkg.stat2);
	return ((smm_pkg.cmd == 0x44494147 /*DIAG*/) &&
			(smm_pkg.stat2 == 0x44454C4C /*DELL*/));
}

bool SMIMonitor::i8k_get_dell_signature() {
	return (i8k_get_dell_sig_aux(I8K_SMM_GET_DELL_SIG1) ||
		    i8k_get_dell_sig_aux(I8K_SMM_GET_DELL_SIG2));
}

/*
 * Read the fan speed in RPM.
 */
int SMIMonitor::i8k_get_fan_speed(int fan, bool force_access) {
	SMBIOS_PKG smm_pkg {};
	int rc;
	int speed = 0;

	smm_pkg.cmd = I8K_SMM_GET_SPEED;
	smm_pkg.data = fan & 0xff;
	if ((rc=i8k_smm(&smm_pkg, force_access)) < 0) {
		return rc;
	}
	speed = (smm_pkg.cmd & 0xffff) * fanMult;
	return speed;
}

/*
 * Read the fan status.
 */
int SMIMonitor::i8k_get_fan_status(int fan) {
	SMBIOS_PKG smm_pkg {};
	int rc;

	smm_pkg.cmd = I8K_SMM_GET_FAN;
	smm_pkg.data = fan & 0xff;
	if ((rc=i8k_smm(&smm_pkg, true)) < 0) {
		return rc;
	}

	return (smm_pkg.cmd & 0xff);
}

/*
 * Read the fan status.
 */
int SMIMonitor::i8k_get_fan_type(int fan) {
	SMBIOS_PKG smm_pkg {};
	int rc;

	smm_pkg.cmd = I8K_SMM_GET_FAN_TYPE;
	smm_pkg.data = fan & 0xff;
	if ((rc=i8k_smm(&smm_pkg, true)) < 0) {
		return rc;
	}

	return (smm_pkg.cmd & 0xff);
}


/*
 * Read the fan nominal rpm for specific fan speed (0,1,2) or zero
 */
int SMIMonitor::i8k_get_fan_nominal_speed(int fan, int speed) {
	SMBIOS_PKG smm_pkg {};
	smm_pkg.cmd = I8K_SMM_GET_NOM_SPEED;
	smm_pkg.data = (fan & 0xff) | (speed << 8);
	return i8k_smm(&smm_pkg, true) ? 0 : (smm_pkg.cmd & 0xffff) * fanMult;
}

/*
 * Set the fan speed (off, low, high). Returns the new fan status.
 */
int SMIMonitor::i8k_set_fan(int fan, int speed) {
	SMBIOS_PKG smm_pkg {};
	smm_pkg.cmd = I8K_SMM_SET_FAN;

	speed = (speed < 0) ? 0 : ((speed > I8K_FAN_MAX) ? I8K_FAN_MAX : speed);
	smm_pkg.data = (fan & 0xff) | (speed << 8);

	return i8k_smm(&smm_pkg, true);
}

int SMIMonitor::i8k_set_fan_control_manual() {
	// we have to write to both control registers since some Dell models
	// support only one register and smm does not return error for unsupported one
	SMBIOS_PKG smm_pkg {};
	smm_pkg.cmd = I8K_SMM_IO_DISABLE_FAN_CTL2;
	smm_pkg.data = 0;
	int result1 = i8k_smm(&smm_pkg, true);
	
	smm_pkg = {};
	smm_pkg.cmd = I8K_SMM_IO_DISABLE_FAN_CTL2;
	smm_pkg.data = 0;
	int result2 = i8k_smm(&smm_pkg, true);

	return (result1 >= 0) ? result1 : result2;
}

int SMIMonitor::i8k_set_fan_control_auto() {
	// we have to write to both control registers since some Dell models
	// support only one register and smm does not return error for unsupported one
	SMBIOS_PKG smm_pkg {};
	smm_pkg.cmd = I8K_SMM_IO_ENABLE_FAN_CTL2;
	smm_pkg.data = 0;
	int result1 = i8k_smm(&smm_pkg, true);
	
	smm_pkg = {};
	smm_pkg.cmd = I8K_SMM_IO_ENABLE_FAN_CTL2;
	smm_pkg.data = 0;
	int result2 =  i8k_smm(&smm_pkg, true);

	return (result1 >= 0) ? result1 : result2;
}

void SMIMonitor::createShared() {
	if (instance)
		PANIC("sdell", "attempted to allocate smi monitor again");
	instance = new SMIMonitor;
	if (!instance)
		PANIC("sdell", "failed to allocate smi monitor");
	instance->mainLock = IOLockAlloc();
	if (!instance->mainLock)
		PANIC("sdell", "failed to allocate smi monitor main lock");
	instance->queueLock = IOSimpleLockAlloc();
	if (!instance->queueLock)
		PANIC("sdell", "failed to allocate simple lock");
	// Reserve SMC updates slots
	if (!instance->storedSmcUpdates.reserve(MaxActiveSmcUpdates))
		PANIC("sdell", "failed to reserve SMC updates slots");
	instance->preemptionLock = IOSimpleLockAlloc();
	if (!instance->preemptionLock)
		PANIC("sdell", "failed to allocate simple lock");
}

bool SMIMonitor::probe() {

	bool success = true;
	wmiDevice = WMIDellDevice::probe();
	if (wmiDevice == nullptr) {
		SYSLOG("sdell", "WMIDellDevice could not be created.");
		return false;
	}

	while (!updateCall) {
		updateCall = thread_call_allocate(staticUpdateThreadEntry, this);
		if (!updateCall) {
			DBGLOG("sdell", "Update thread cannot be created");
			success = false;
			break;
		}

		IOLockLock(mainLock);
		thread_call_enter(updateCall);
		
		while (initialized == -1) {
			IOLockSleep(mainLock, &initialized, THREAD_UNINT);
		}
		IOLockUnlock(mainLock);

		if (initialized != KERN_SUCCESS) {
			success = false;
			break;
		}

		DBGLOG("sdell", "found %u fan sensors and %u temp sensors", fanCount, tempCount);
		break;
	}

	if (!success) {
		if (updateCall) {
			while (!thread_call_free(updateCall))
				thread_call_cancel(updateCall);
			updateCall = nullptr;
		}
	}

	return success;
}

void SMIMonitor::start() {
	DBGLOG("sdell", "Based on I8kfan project and adopted to VirtualSMC plugin");
}

void SMIMonitor::handlePowerOff() {
	if (awake) {
		ignore_new_smc_updates = true;
		if (fansStatus != 0) {
			// turn off manual control for all fans
			UInt16 data = 0;
			postSmcUpdate(KeyFS__, -1, &data, sizeof(data), true);
		}
		DBGLOG("sdell", "SMIMonitor switched to sleep state, smc updates before sleep: %lu", storedSmcUpdates.size());
		while (storedSmcUpdates.size() != 0) { IOSleep(10); }
		awake = false;
	}
}

void SMIMonitor::handlePowerOn() {
	if (!awake) {
		awake = true;
		// turn off manual control for all fans
		UInt16 data = 0;
		postSmcUpdate(KeyFS__, -1, &data, sizeof(data), true);
		ignore_new_smc_updates = false;
		DBGLOG("sdell", "SMIMonitor switched to awake state");
		while (storedSmcUpdates.size() != 0) { IOSleep(10); }
	}
}

bool SMIMonitor::postSmcUpdate(SMC_KEY key, size_t index, const void *data, uint32_t dataSize, bool force_update)
{
	if (!force_update && (!awake || ignore_new_smc_updates)) {
		DBGLOG("sdell", "SMIMonitor: postSmcUpdate for key %d has been ignored", key);
		return false;
	}
	
	IOSimpleLockLock(queueLock);

	bool success = false;
	while (1) {

		if (dataSize > sizeof(StoredSmcUpdate::data)) {
			SYSLOG("sdell", "postRequest dataSize overflow %u", dataSize);
			break;
		}

		for (size_t i = 0; i < storedSmcUpdates.size() && !success; i++) {
			auto &si = storedSmcUpdates[i];
			if (si.key == key && si.index == index) {
				si.size = dataSize;
				if (dataSize > 0)
					lilu_os_memcpy(si.data, data, dataSize);
				success = true;
				break;
			}
		}

		if (success) break;

		if (storedSmcUpdates.size() == MaxActiveSmcUpdates) {
			SYSLOG("sdell", "postRequest reserve overflow");
			break;
		}

		StoredSmcUpdate si {key, index, dataSize};
		if (dataSize > 0)
			lilu_os_memcpy(si.data, data, dataSize);
		if (storedSmcUpdates.push_back(si)) {
			success = true;
			break;
		}
		break;
	}
	IOSimpleLockUnlock(queueLock);

	if (!success)
		SYSLOG("sdell", "unable to store smc update");

	return success;
}

IOReturn SMIMonitor::bindCurrentThreadToCpu0()
{
	// Obtain power management callbacks 10.7+
	pmCallBacks_t callbacks {};
	pmKextRegister(PM_DISPATCH_VERSION, nullptr, &callbacks);

	if (!callbacks.LCPUtoProcessor) {
		SYSLOG("sdell", "failed to obtain LCPUtoProcessor");
		return KERN_FAILURE;
	}

	if (!callbacks.ThreadBind) {
		SYSLOG("sdell", "failed to obtain ThreadBind");
		return KERN_FAILURE;
	}

	if (!IOSimpleLockTryLock(preemptionLock)) {
		SYSLOG("sdell", "Preemption cannot be disabled before performing ThreadBind");
		return KERN_FAILURE;
	}

	bool success = true;
	auto enable = ml_set_interrupts_enabled(FALSE);

	while (1)
	{
		auto processor = callbacks.LCPUtoProcessor(0);
		if (processor == nullptr) {
			SYSLOG("sdell", "failed to call LCPUtoProcessor with cpu 0");
			success = false;
			break;
		}
		callbacks.ThreadBind(processor);
		break;
	}

	IOSimpleLockUnlock(preemptionLock);
	ml_set_interrupts_enabled(enable);
	
	return success ? KERN_SUCCESS : KERN_FAILURE;
}

bool SMIMonitor::findFanSensors() {
	fanCount = 0;

	for (int i = 0; i < state.MaxFanSupported; i++) {
		state.fanInfo[fanCount] = {};
		
		uint32_t addr = static_cast<uint32_t>(WMI_TOKEN::Fan1Rpm) + i*0x10;
		int_array args = {addr, 0, 0, 0}, res = {0, 0, 0, 0};
		if (wmiDevice->evaluate(WMI_CLASS::TokenRead, WMI_SELECTOR::Standard, args, res) && res[0] == 0)
		{
			state.fanInfo[fanCount].index = i;
			state.fanInfo[fanCount].speed = res[1];
			state.fanInfo[fanCount].status = 0;
			state.fanInfo[fanCount].minSpeed = 2200;
			state.fanInfo[fanCount].maxSpeed = 4900;
			state.fanInfo[fanCount].wmi_addr = addr;
			state.fanInfo[fanCount].smm = false;
			DBGLOG("sdell", "WMI Fan %d has been detected (addr = 0x%04X, speed=%d, minSpeed=%d, maxSpeed=%d", i, addr,
				   state.fanInfo[fanCount].speed, state.fanInfo[fanCount].minSpeed, state.fanInfo[fanCount].maxSpeed);
		}
			
		int rc = i8k_get_fan_status(i);
		if (rc < 0) {
			IOSleep(100);
			rc = i8k_get_fan_status(i);
		}
		if (rc >= 0) {
			rc = i8k_get_fan_speed(i, true);
			if (rc >= 0) {
				state.fanInfo[fanCount].index = i;
				state.fanInfo[fanCount].speed = rc;
				state.fanInfo[fanCount].minSpeed = i8k_get_fan_nominal_speed(i, I8K_FAN_LOW);
				state.fanInfo[fanCount].maxSpeed = i8k_get_fan_nominal_speed(i, I8K_FAN_HIGH);
				state.fanInfo[fanCount].smm = true;
				rc = i8k_get_fan_status(i);
				if (rc >= 0)
					state.fanInfo[i].status = rc;
				int type = i8k_get_fan_type(i);
				if ((type > FanInfo::Unsupported) && (type < FanInfo::Last))
					state.fanInfo[fanCount].type = static_cast<FanInfo::FanType>(type);
				DBGLOG("sdell", "SMM Fan %d has been detected, type=%d, status=%d, speed=%d, minSpeed=%d, maxSpeed=%d", i, type,
					   state.fanInfo[fanCount].status, state.fanInfo[fanCount].speed, state.fanInfo[fanCount].minSpeed, state.fanInfo[fanCount].maxSpeed);
			}
		}
		
		if (state.fanInfo[fanCount].wmi_addr != 0 || state.fanInfo[fanCount].smm)
			fanCount++;
	}
	
	return fanCount > 0;
}

bool SMIMonitor::findTempSensors() {
	tempCount = 0;
	
	for (int i=0; i<state.MaxTempSupported; i++)
	{
		state.tempInfo[i] = {};
		int rc = i8k_get_temp(i, true);
		if (rc < 0) {
			IOSleep(100);
			rc = i8k_get_temp(i, true);
		}
		if (rc >= 0)
		{
			state.tempInfo[tempCount].index = i;
			state.tempInfo[i].temp = rc;
			uint type = i8k_get_temp_type(i);
			if (type <= TempInfo::Unsupported || type >= TempInfo::Last) {
				DBGLOG("sdell", "Temp sensor type %d is unknown, auto assign value %d", type, state.tempInfo[i].index);
				state.tempInfo[i].type = static_cast<TempInfo::SMMTempSensorType>(state.tempInfo[i].index);
			}
			DBGLOG("sdell", "Temp sensor %d has been detected, type %d, temp = %d", i, state.tempInfo[tempCount].type, rc);
			tempCount++;
		}
	}
	
	return tempCount > 0;
}

void SMIMonitor::staticUpdateThreadEntry(thread_call_param_t param0, thread_call_param_t param1)
{
	auto *that = OSDynamicCast(SMIMonitor, reinterpret_cast<OSObject*>(param0));
	if (!that) {
		SYSLOG("sdell", "Failed to get pointer to SMIMonitor");
		return;
	}

	bool success = true;
	while (true) {
			
		IOReturn result = that->bindCurrentThreadToCpu0();
		if (result != KERN_SUCCESS) {
			success = false;
			break;
		}
		
		IOSleep(500);
		
		auto enable = ml_set_interrupts_enabled(FALSE);
		auto cpu_n = cpu_number();
		ml_set_interrupts_enabled(enable);
		
		if (cpu_n != 0) {
			DBGLOG("sdell", "staticUpdateThreadEntry is called in context CPU %d", cpu_n);
			success = false;
			break;
		}
		
		if (!that->i8k_get_dell_signature()) {
			SYSLOG("sdell", "Unable to get Dell SMM signature!");
		}
		
		if (!that->findFanSensors() || !that->findTempSensors()) {
			SYSLOG("sdell", "failed to find fans or temp sensors!");
			success = false;
			break;
		}
		
		break;
	}
	
	IOLockLock(that->mainLock);
	that->initialized = success ? KERN_SUCCESS : KERN_FAILURE;
	IOLockWakeup(that->mainLock, &that->initialized, true);
	IOLockUnlock(that->mainLock);
	
	if (success)
		that->updateSensorsLoop();
}

void SMIMonitor::updateSensorsLoop() {
	
	//i8k_set_fan_control_auto(); // force automatic control

	while (true) {
				
		for (int i=0; i<fanCount && awake; ++i)
		{
			if (state.fanInfo[i].wmi_addr != 0) {
				
				int_array args = {state.fanInfo[i].wmi_addr,0,0,0}, res = {0,0,0,0};
				if (wmiDevice->evaluate(WMI_CLASS::TokenRead, WMI_SELECTOR::Standard, args, res))
				{
					if (res[0] != 0)
						SYSLOG("sdell", "WMI interface returned error: %d", res[0]);
					else
						state.fanInfo[i].speed = res[1];
				}
				else
					SYSLOG("sdell", "WMI evaluate has failed");
			}
			else if (state.fanInfo[i].smm)
			{
				int sensor = state.fanInfo[i].index;
				int rc = i8k_get_fan_speed(sensor, false);
				if (rc >= 0)
					state.fanInfo[i].speed = rc;
				else
					DBGLOG("sdell", "SMM reading error %d for fan %d", rc, sensor);
			}
		}
				
		handleSmcUpdatesInIdle(50);
	}
}

void SMIMonitor::handleManualUpdateAllSensors()
{
	for (int i=0; i<fanCount && awake; ++i)
	{
		if (state.fanInfo[i].wmi_addr != 0) {
			
			int_array args = {state.fanInfo[i].wmi_addr,0,0,0}, res = {0,0,0,0};
			if (wmiDevice->evaluate(WMI_CLASS::TokenRead, WMI_SELECTOR::Standard, args, res))
			{
				if (res[0] != 0)
					SYSLOG("sdell", "WMI interface returned error: %d", res[0]);
				else
					state.fanInfo[i].speed = res[1];
			}
			else
				SYSLOG("sdell", "WMI evaluate has failed");
		}
		else if (state.fanInfo[i].smm)
		{
			int sensor = state.fanInfo[i].index;
			int rc = i8k_get_fan_speed(sensor, false);
			if (rc >= 0)
				state.fanInfo[i].speed = rc;
			else
				DBGLOG("sdell", "SMM reading error %d for fan %d", rc, sensor);
			rc = i8k_get_fan_status(sensor);
			if (rc >= 0)
				state.fanInfo[i].status = rc;
		}
	}
	
	for (int i=0; i<tempCount && awake; ++i)
	{
		int sensor = state.tempInfo[i].index;
		int rc = i8k_get_temp(sensor);
		if (rc >= 0)
			state.tempInfo[i].temp = rc;
		else
			DBGLOG("sdell", "SMM reading error %d for temp sensor %d", rc, sensor);
	}
}

void SMIMonitor::handleSmcUpdatesInIdle(int idle_loop_count)
{
	for (int i=0; i<idle_loop_count; ++i)
	{
		if (awake && storedSmcUpdates.size() != 0) {
			IOSimpleLockLock(queueLock);
			if (storedSmcUpdates.size() > 0) {
				StoredSmcUpdate update = storedSmcUpdates[0];
				storedSmcUpdates.erase(0, false);
				IOSimpleLockUnlock(queueLock);

				switch (update.key) {
					case KeyF0Md:
						hanldeManualControlUpdate(update.index, update.data);
						break;
					case KeyF0Tg:
						hanldeManualTargetFanSpeedUpdate(update.index, update.data);
						break;
					case KeyFS__:
						handleManualForceFanControlUpdate(update.data);
						break;
					case KeyRFSH:
						handleManualUpdateAllSensors();
						break;
					case KeyFaMD:
						hanldeManualTargetFanModeUpdate(update.index, update.data);
						break;
				}
			}
			else {
				IOSimpleLockUnlock(queueLock);
			}
		}

		IOSleep(100);
	}
}

void SMIMonitor::hanldeManualControlUpdate(size_t index, UInt8 *data)
{
	UInt16 val = data[0];
	int rc = 0;

	auto newStatus = val ? (fansStatus | (1 << index)) : (fansStatus & ~(1 << index));
	if (fansStatus != newStatus) {
		if (fansStatus == 0 && newStatus != 0)
		{
			rc = i8k_set_fan_control_manual();
			DBGLOG("sdell", "call i8k_set_fan_control_manual, rc = %d", rc);
		}
		else if (fansStatus != 0 && newStatus == 0)
		{
			rc = i8k_set_fan_control_auto();
			DBGLOG("sdell", "call i8k_set_fan_control_auto, rc = %d", rc);
		}
		fansStatus = newStatus;
		DBGLOG("sdell", "Set manual mode for fan %lu to %s, global fansStatus = 0x%02x", index, val ? "enable" : "disable", fansStatus);
	}

	if (rc != 0)
		SYSLOG("sdell", "Set manual mode for fan %lu to %d failed: %d", index, val, rc);
}

void SMIMonitor::hanldeManualTargetFanSpeedUpdate(size_t index, UInt8 *data)
{
	auto value = VirtualSMCAPI::decodeIntFp(SmcKeyTypeFpe2, *reinterpret_cast<const uint16_t *>(data));
	state.fanInfo[index].targetSpeed = value;
	if (!state.fanInfo[index].smm)
		return;

	if (fansStatus & (1 << index)) {
		int status = 1;
		int range = state.fanInfo[index].maxSpeed - state.fanInfo[index].minSpeed;
		if (value > state.fanInfo[index].minSpeed + range/2)
			status = 2;
		else if (state.fanInfo[index].stopOffset != 0 && value < (state.fanInfo[index].minSpeed + state.fanInfo[index].stopOffset))
			status = 0;		// stop fan
		
		int current_status = i8k_get_fan_status(state.fanInfo[index].index);
		state.fanInfo[index].status = current_status;
		if (current_status < 0 || current_status != status) {
			int rc = i8k_set_fan(state.fanInfo[index].index, status);
			if (rc != 0)
				SYSLOG("sdell", "Set target speed for fan %lu to %d failed: %d", index, value, rc);
			else {
				state.fanInfo[index].status = status;
				DBGLOG("sdell", "Set target speed for fan %lu to %d, status = %d", index, value, status);
			}
		}
	}
	else
		SYSLOG("sdell", "Set target speed for fan %lu to %d ignored since auto control is active", index, value);
}

void SMIMonitor::hanldeManualTargetFanModeUpdate(size_t index, UInt8 *data)
{
	if (!state.fanInfo[index].smm)
		return;
	int status = *data;
	int rc = i8k_set_fan(state.fanInfo[index].index, status);
	if (rc != 0)
		SYSLOG("sdell", "Set target mode for fan %lu to %d failed: %d", index, status, rc);
	else {
		state.fanInfo[index].status = status;
		DBGLOG("sdell", "Set target mode for fan %lu to %d", index, status);
	}
}

void SMIMonitor::handleManualForceFanControlUpdate(UInt8 *data)
{
	auto val = (data[0] << 8) + data[1]; //big endian data

	int rc = 0;

	if (fansStatus != val) {
		if (fansStatus == 0 && val != 0)
		{
			rc = i8k_set_fan_control_manual();
			DBGLOG("sdell", "call i8k_set_fan_control_manual, rc = %d", rc);
		}
		else if (fansStatus != 0 && val == 0)
		{
			rc = i8k_set_fan_control_auto();
			DBGLOG("sdell", "call i8k_set_fan_control_auto, rc = %d", rc);
		}
		fansStatus = val;
		DBGLOG("sdell", "Set force fan mode to %d", val);
	}
	
	if (rc != 0)
		SYSLOG("sdell", "Set force fan mode to %d failed: %d", val, rc);
}
