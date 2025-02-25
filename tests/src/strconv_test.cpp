#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "ocpp/strconv.h"

TEST_GROUP(strconv) {
	void setup(void) {
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(strconv, ShouldReturnMeasurand_WhenFreqStringGiven) {
	const ocpp_measurand_t y = ocpp_get_measurand_from_string("Frequency", 9);
	LONGS_EQUAL(OCPP_MEASURAND_FREQUENCY, y);
}

TEST(strconv, ShouldReturnZero_WhenInvalidMeasurandStringGiven) {
	const ocpp_measurand_t y = ocpp_get_measurand_from_string("Invalid", 7);
	LONGS_EQUAL(0, y);
}

TEST(strconv, ShouldReturnAuthStatus_WhenAuthStatusStringGiven) {
	const ocpp_auth_status_t y = ocpp_get_auth_status_from_string("Accepted");
	LONGS_EQUAL(OCPP_AUTH_STATUS_ACCEPTED, y);
}

TEST(strconv, ShouldReturnBootStatus_WhenBootStatusStringGiven) {
	const ocpp_boot_status_t y = ocpp_get_boot_status_from_string("Accepted");
	LONGS_EQUAL(OCPP_BOOT_STATUS_ACCEPTED, y);
}

TEST(strconv, ShouldReturnStatusString_WhenStatusGiven) {
	const char *y = ocpp_stringify_status(OCPP_STATUS_SUSPENDED_EV);
	STRCMP_EQUAL("SuspendedEV", y);
}

TEST(strconv, ShouldReturnCommStatusString_WhenCommStatusGiven) {
	const char *y = ocpp_stringify_comm_status(OCPP_COMM_DOWNLOADED);
	STRCMP_EQUAL("Downloaded", y);
}

TEST(strconv, ShouldReturnErrorString_WhenErrorGiven) {
	const char *y = ocpp_stringify_error(OCPP_ERROR_INTERNAL);
	STRCMP_EQUAL("InternalError", y);
}

TEST(strconv, ShouldReturnAvailabilityStatusString_WhenAvailabilityStatusGiven) {
	const char *y = ocpp_stringify_availability_status(OCPP_AVAILABILITY_STATUS_ACCEPTED);
	STRCMP_EQUAL("Accepted", y);
}

TEST(strconv, ShouldReturnConfigStatusString_WhenConfigStatusGiven) {
	const char *y = ocpp_stringify_config_status(OCPP_CONFIG_STATUS_ACCEPTED);
	STRCMP_EQUAL("Accepted", y);
}

TEST(strconv, ShouldReturnDataStatusString_WhenDataStatusGiven) {
	const char *y = ocpp_stringify_data_status(OCPP_DATA_STATUS_ACCEPTED);
	STRCMP_EQUAL("Accepted", y);
}

TEST(strconv, ShouldReturnProfileStatusString_WhenProfileStatusGiven) {
	const char *y = ocpp_stringify_profile_status(OCPP_PROFILE_STATUS_ACCEPTED);
	STRCMP_EQUAL("Accepted", y);
}

TEST(strconv, ShouldReturnRemoteStatusString_WhenRemoteStatusGiven) {
	const char *y = ocpp_stringify_remote_status(OCPP_REMOTE_STATUS_ACCEPTED);
	STRCMP_EQUAL("Accepted", y);
}

TEST(strconv, ShouldReturnTriggerStatusString_WhenTriggerStatusGiven) {
	const char *y = ocpp_stringify_trigger_status(OCPP_TRIGGER_STATUS_ACCEPTED);
	STRCMP_EQUAL("Accepted", y);
}

TEST(strconv, ShouldReturnStopReasonString_WhenStopReasonGiven) {
	const char *y = ocpp_stringify_stop_reason(OCPP_STOP_REASON_EMERGENCY_STOP);
	STRCMP_EQUAL("EmergencyStop", y);
}

TEST(strconv, ShouldReturnMeasurandString_WhenMeasurandGiven) {
	const char *y = ocpp_stringify_measurand(OCPP_MEASURAND_FREQUENCY);
	STRCMP_EQUAL("Frequency", y);
}
