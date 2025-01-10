#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "ocpp/core/configuration.h"
#include "ocpp/overrides.h"
#include <errno.h>

int ocpp_configuration_lock(void) {
	return 0;
}
int ocpp_configuration_unlock(void) {
	return 0;
}

TEST_GROUP(Configuration) {
	void setup(void) {
		ocpp_reset_configuration();
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}
};

TEST(Configuration, reset_ShouldSetTheDefaultValues) {
	int connTimeout;
	bool authRemoteTxReq;
	int blink;

	ocpp_get_configuration("ConnectionTimeOut", &connTimeout, sizeof(connTimeout), NULL);
	ocpp_get_configuration("AuthorizeRemoteTxRequests", &authRemoteTxReq, sizeof(authRemoteTxReq), NULL);
	ocpp_get_configuration("BlinkRepeat", &blink, sizeof(blink), NULL);

	LONGS_EQUAL(180, connTimeout);
	LONGS_EQUAL(true, authRemoteTxReq);
	LONGS_EQUAL(0, blink);
}

TEST(Configuration, has_configuration_ShouldReturnTrue_WhenValidOneGiven) {
	LONGS_EQUAL(true, ocpp_has_configuration("AuthorizeRemoteTxRequests"));
	LONGS_EQUAL(true, ocpp_has_configuration("WebSocketPingInterval"));
}

TEST(Configuration, has_configuration_ShouldReturnFalse_WhenInvalidOneGiven) {
	LONGS_EQUAL(false, ocpp_has_configuration("AuthorizeRemoteTxRequest"));
	LONGS_EQUAL(false, ocpp_has_configuration("UnkwonKey"));
}

TEST(Configuration, count_ShouldReturnTheNumberOfConfigurations) {
	LONGS_EQUAL(55, ocpp_count_configurations());
}

TEST(Configuration, get_len_ShouldReturnWholeConfigurationSize) {
	LONGS_EQUAL(271, ocpp_compute_configuration_size());
}

TEST(Configuration, set_ShouldSetTheConfiguration) {
	int expected = 3;
	int actual = 0;
	ocpp_set_configuration("ResetRetries", &expected, sizeof(expected));
	ocpp_get_configuration("ResetRetries", &actual, sizeof(actual), NULL);
	LONGS_EQUAL(3, actual);
	expected = OCPP_MEASURAND_SOC | OCPP_MEASURAND_TEMPERATURE;
	ocpp_set_configuration("MeterValuesSampledData", &expected, sizeof(expected));
	ocpp_get_configuration("MeterValuesSampledData", &actual, sizeof(actual), NULL);
	LONGS_EQUAL(0x180000, actual);
}

IGNORE_TEST(Configuration, get_ShouldReturnEACCES_WhenWriteOnlyGiven) {
	char buf[16];
	LONGS_EQUAL(-EACCES, ocpp_get_configuration("AuthorizationKey", buf, sizeof(buf), NULL));
}

TEST(Configuration, is_writable_ShouldReturnItsAccessibility) {
	LONGS_EQUAL(false, ocpp_is_configuration_writable("NumberOfConnectors"));
	LONGS_EQUAL(true, ocpp_is_configuration_writable("StopTransactionOnInvalidId"));
	LONGS_EQUAL(false, ocpp_is_configuration_writable("UnknownKey"));
}

TEST(Configuration, get_ShouldReturnEINVAL_WhenUnknownKeyGiven) {
	int tmp;
	LONGS_EQUAL(-EINVAL, ocpp_get_configuration("AnyKey", &tmp, sizeof(tmp), NULL));
}

TEST(Configuration, set_ShouldReturnEINVAL_WhenUnknownKeyGiven) {
	int tmp;
	LONGS_EQUAL(-EINVAL, ocpp_set_configuration("AnyKey", &tmp, sizeof(tmp)));
}

TEST(Configuration, set_ShouldReturnEPERM_WhenTryWriteOnReadOnly) {
	int tmp;
	LONGS_EQUAL(-EPERM, ocpp_set_configuration("NumberOfConnectors", &tmp, sizeof(tmp)));
}

TEST(Configuration, type_and_size_ShouldReturnConfigurationTypeAndSize_WhenBOOLTypeGiven) {
	LONGS_EQUAL(OCPP_CONF_TYPE_BOOL, ocpp_get_configuration_data_type("AllowOfflineTxForUnknownId"));
	LONGS_EQUAL(sizeof(bool), ocpp_get_configuration_size("AllowOfflineTxForUnknownId"));
}

TEST(Configuration, type_and_size_ShouldReturnConfigurationTypeAndSize_WhenINTTypeGiven) {
	LONGS_EQUAL(OCPP_CONF_TYPE_INT, ocpp_get_configuration_data_type("ConnectionTimeOut"));
	LONGS_EQUAL(sizeof(int), ocpp_get_configuration_size("ConnectionTimeOut"));
}

TEST(Configuration, type_and_size_ShouldReturnConfigurationTypeAndSize_WhenSTRTypeGiven) {
	LONGS_EQUAL(OCPP_CONF_TYPE_STR, ocpp_get_configuration_data_type("AuthorizationKey"));
	LONGS_EQUAL(40, ocpp_get_configuration_size("AuthorizationKey"));
}

TEST(Configuration, type_and_size_ShouldReturnConfigurationTypeAndSize_WhenCSLTypeGiven) {
	LONGS_EQUAL(OCPP_CONF_TYPE_CSL, ocpp_get_configuration_data_type("ConnectorPhaseRotation"));
	LONGS_EQUAL(sizeof(int), ocpp_get_configuration_size("ConnectorPhaseRotation"));
}

TEST(Configuration, type_and_size_ShouldReturnConfigurationTypeAndSize_WhenUnknownTypeGiven) {
	LONGS_EQUAL(OCPP_CONF_TYPE_UNKNOWN, ocpp_get_configuration_data_type("UnknownKey"));
	LONGS_EQUAL(0, ocpp_get_configuration_size("UnknownKey"));
}

TEST(Configuration, get_keystr_ShouldReturnKeyString_WhenKnownKeyGiven) {
	STRCMP_EQUAL("HeartbeatInterval", ocpp_get_configuration_keystr_from_index(9));
}

TEST(Configuration, get_keystr_ShouldReturnUnknownKeyString_WhenUnknownKeyGiven) {
	STRCMP_EQUAL(NULL, ocpp_get_configuration_keystr_from_index(-1));
}
