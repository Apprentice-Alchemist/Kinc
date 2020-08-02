#include "pch.h"

#ifdef KORE_VR

#include "VrInterface.h"

#include <string.h>
#include <kinc/vr/vrinterface.h>

using namespace Kore;

SensorState sensorState;

void* VrInterface::init(void* hinst, const char* title, const char* windowClassName) {
	kinc_vr_interface_init(hinst, title, windowClassName);
}

void VrInterface::begin() {
	kinc_vr_interface_begin();
}

void VrInterface::beginRender(int eye) {
	kinc_vr_interface_begin_render(eye);
}

void VrInterface::endRender(int eye) {
	kinc_vr_interface_end_render(eye);
}

SensorState VrInterface::getSensorState(int eye) {
	kinc_vr_sensor_state_t sensor_state = kinc_vr_interface_get_sensor_state(eye);
	sensorState.pose.vrPose.left = sensor_state.pose.vrPose.left;
	sensorState.pose.vrPose.right = sensor_state.pose.vrPose.right;
	sensorState.pose.vrPose.bottom = sensor_state.pose.vrPose.bottom;
	sensorState.pose.vrPose.top = sensor_state.pose.vrPose.top;
	sensorState.pose.vrPose.orientation.x = sensor_state.pose.vrPose.orientation.x;
	sensorState.pose.vrPose.orientation.y = sensor_state.pose.vrPose.orientation.y;
	sensorState.pose.vrPose.orientation.z = sensor_state.pose.vrPose.orientation.z;
	sensorState.pose.vrPose.orientation.w = sensor_state.pose.vrPose.orientation.w;
	sensorState.pose.vrPose.position.set(sensor_state.pose.vrPose.position.x,
										 sensor_state.pose.vrPose.position.y,
										 sensor_state.pose.vrPose.position.z);
	memcpy(sensorState.pose.vrPose.eye.data, sensor_state.pose.vrPose.eye.m, sizeof(sensor_state.pose.vrPose.eye.m));
	memcpy(sensorState.pose.vrPose.projection.data, sensor_state.pose.vrPose.projection.m, sizeof(sensor_state.pose.vrPose.projection.m));
	sensorState.pose.angularVelocity.set(sensor_state.pose.angularVelocity.x,
										 sensor_state.pose.angularVelocity.y,
										 sensor_state.pose.angularVelocity.z);
	sensorState.pose.linearVelocity.set(sensor_state.pose.linearVelocity.x,
										sensor_state.pose.linearVelocity.y,
										sensor_state.pose.linearVelocity.z);
	sensorState.pose.angularAcceleration.set(sensor_state.pose.angularAcceleration.x,
											 sensor_state.pose.angularAcceleration.y,
											 sensor_state.pose.angularAcceleration.z);
	sensorState.pose.linearAcceleration.set(sensor_state.pose.linearAcceleration.x,
											sensor_state.pose.linearAcceleration.y,
											sensor_state.pose.linearAcceleration.z);
	sensorState.pose.trackedDevice = (TrackedDevice)sensor_state.pose.trackedDevice;
	sensorState.pose.isVisible = sensor_state.pose.isVisible;
	sensorState.pose.hmdPresenting = sensor_state.pose.hmdPresenting;
	sensorState.pose.hmdMounted = sensor_state.pose.hmdMounted;
	sensorState.pose.displayLost = sensor_state.pose.displayLost;
	sensorState.pose.shouldQuit = sensor_state.pose.shouldQuit;
	sensorState.pose.shouldRecenter = sensor_state.pose.shouldRecenter;
	return sensorState;
}

void VrInterface::warpSwap() {
	kinc_vr_interface_warp_swap();
}

void VrInterface::updateTrackingOrigin(TrackingOrigin origin) {
	kinc_vr_interface_update_tracking_origin((kinc_tracking_origin_t)origin);
}

void VrInterface::resetHmdPose() {
	kinc_vr_interface_reset_hmd_pose();
}

void VrInterface::ovrShutdown() {
	kinc_vr_interface_ovr_shutdown();
}

#endif
