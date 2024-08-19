#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "ocpp/ocpp.h"
#include "ocpp/overrides.h"
#include <errno.h>
#include <time.h>
#include <stdlib.h>

static struct {
	uint8_t message_id[OCPP_MESSAGE_ID_MAXLEN];
	ocpp_message_role_t role;
	ocpp_message_t type;
} sent;

static struct {
	ocpp_message_role_t role;
	ocpp_message_t type;
} event;

time_t time(time_t *second) {
	return mock().actualCall(__func__).returnUnsignedIntValueOrDefault(0);
}

int ocpp_send(const struct ocpp_message *msg) {
	memcpy(sent.message_id, msg->id, sizeof(sent.message_id));
	sent.role = msg->role;
	sent.type = msg->type;

	return mock().actualCall(__func__).returnIntValueOrDefault(0);
}

int ocpp_recv(struct ocpp_message *msg)
{
	return mock().actualCall(__func__).withOutputParameter("msg", msg).returnIntValueOrDefault(0);
}

int ocpp_lock(void) {
	return 0;
}
int ocpp_unlock(void) {
	return 0;
}

int ocpp_configuration_lock(void) {
	return 0;
}
int ocpp_configuration_unlock(void) {
	return 0;
}

void ocpp_generate_message_id(void *buf, size_t bufsize)
{
	char *p = (char *)buf;
	char charset[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	while (bufsize-- > 0) {
		int index = rand() % (int)(sizeof(charset) - 1);
		*p++ = charset[index];
	}
	*p = '\0';
}

static void on_ocpp_event(ocpp_event_t event_type,
		const struct ocpp_message *msg, void *ctx) {
	event.role = msg->role;
	event.type = msg->type;
	mock().actualCall(__func__).withParameter("event_type", event_type);
}

TEST_GROUP(Core) {
	void setup(void) {
		srand((unsigned int)clock());
		ocpp_init(on_ocpp_event, NULL);
	}
	void teardown(void) {
		mock().checkExpectations();
		mock().clear();
	}

	void step(int sec) {
		mock().expectOneCall("time").andReturnValue(sec);
		ocpp_step();
	}
	void check_tx(ocpp_message_role_t role, ocpp_message_t type) {
		LONGS_EQUAL(role, sent.role);
		LONGS_EQUAL(type, sent.type);
	}
	void check_rx(ocpp_message_role_t role, ocpp_message_t type) {
		LONGS_EQUAL(role, event.role);
		LONGS_EQUAL(type, event.type);
	}
};

TEST(Core, step_ShouldNeverDropBootNotification_WhenSendFailed) {
	ocpp_send_bootnotification(&(const struct ocpp_BootNotification) {
		.chargePointModel = "Model",
		.chargePointVendor = "Vendor",
	});

	int interval;
	ocpp_get_configuration("HeartbeatInterval", &interval, sizeof(interval), 0);

	for (int i = 0; i < 100; i++) {
		mock().expectOneCall("ocpp_recv").ignoreOtherParameters().andReturnValue(-ENOMSG);
		mock().expectOneCall("ocpp_send").andReturnValue(-1);
		step(interval*i);
	}
}

TEST(Core, step_ShouldDropMessage_WhenFailedSendingMoreThanRetries) {
	ocpp_send_datatransfer(&(const struct ocpp_DataTransfer) {
		.vendorId = "VendorID",
	});

	mock().expectOneCall("ocpp_recv").ignoreOtherParameters().andReturnValue(-ENOMSG);
	mock().expectOneCall("ocpp_send").andReturnValue(-1);
	step(0);
	mock().expectOneCall("ocpp_recv").ignoreOtherParameters().andReturnValue(-ENOMSG);
	mock().expectOneCall("ocpp_send").andReturnValue(-1);
	step(OCPP_DEFAULT_TX_TIMEOUT_SEC);
	mock().expectOneCall("ocpp_recv").ignoreOtherParameters().andReturnValue(-ENOMSG);
	step(OCPP_DEFAULT_TX_TIMEOUT_SEC*2);
}

TEST(Core, step_ShouldSendHeartBeat_WhenNoMessageSentDuringHeartBeatInterval) {
	int interval;
	ocpp_get_configuration("HeartbeatInterval", &interval, sizeof(interval), NULL);
	mock().expectOneCall("ocpp_recv").ignoreOtherParameters().andReturnValue(-ENOMSG);
	mock().expectOneCall("ocpp_send").andReturnValue(0);
	step(interval);
	check_tx(OCPP_MSG_ROLE_CALL, OCPP_MSG_HEARTBEAT);

	struct ocpp_message resp = {
		.role = OCPP_MSG_ROLE_CALLRESULT,
		.type = OCPP_MSG_HEARTBEAT,
	};
	memcpy(resp.id, sent.message_id, sizeof(sent.message_id));
	mock().expectOneCall("ocpp_recv").withOutputParameterReturning("msg", &resp, sizeof(resp));
	mock().expectOneCall("on_ocpp_event").withParameter("event_type", 2);
	mock().expectOneCall("on_ocpp_event").withParameter("event_type", 0);
	step(interval*2-1);
	check_rx(OCPP_MSG_ROLE_CALLRESULT, OCPP_MSG_HEARTBEAT);
	mock().expectOneCall("ocpp_recv").ignoreOtherParameters().andReturnValue(-ENOMSG);
	mock().expectOneCall("ocpp_send").andReturnValue(0);
	step(interval*2);
}

TEST(Core, step_ShouldNotSendHeartBeat_WhenAnyMessageSentDuringHeartBeatInterval) {
	int interval;
	ocpp_get_configuration("HeartbeatInterval", &interval, sizeof(interval), NULL);
	ocpp_send_datatransfer(&(const struct ocpp_DataTransfer) {
		.vendorId = "VendorID",
	});
	mock().expectOneCall("ocpp_send").andReturnValue(0);
	mock().expectOneCall("ocpp_recv").ignoreOtherParameters().andReturnValue(-ENOMSG);
	step(interval);
	check_tx(OCPP_MSG_ROLE_CALL, OCPP_MSG_DATA_TRANSFER);
	
	struct ocpp_message resp = {
		.role = OCPP_MSG_ROLE_CALLRESULT,
		.type = sent.type,
	};
	memcpy(resp.id, sent.message_id, sizeof(sent.message_id));
	mock().expectOneCall("ocpp_recv").withOutputParameterReturning("msg", &resp, sizeof(resp));
	mock().expectOneCall("on_ocpp_event").withParameter("event_type", 2);
	mock().expectOneCall("on_ocpp_event").withParameter("event_type", 0);
	step(interval*2-1);
	check_rx(OCPP_MSG_ROLE_CALLRESULT, OCPP_MSG_DATA_TRANSFER);
}

TEST(Core, ShouldSendStartTransaction_WhenQueueIsFull) {
	int interval;
	ocpp_get_configuration("HeartbeatInterval", &interval, sizeof(interval), NULL);
	struct ocpp_DataTransfer msg[8];
	struct ocpp_StartTransaction start;
	for (int i = 0; i < 8; i++) {
		LONGS_EQUAL(0, ocpp_push_request(OCPP_MSG_DATA_TRANSFER, &msg[i], sizeof(msg[i]), false));
	}

	LONGS_EQUAL(-ENOMEM, ocpp_push_request(OCPP_MSG_START_TRANSACTION, &start, sizeof(start), false));
	mock().expectOneCall("on_ocpp_event").withParameter("event_type", OCPP_EVENT_MESSAGE_FREE);
	LONGS_EQUAL(0, ocpp_push_request(OCPP_MSG_START_TRANSACTION, &start, sizeof(start), true));

	mock().expectNCalls(6, "on_ocpp_event").withParameter("event_type", OCPP_EVENT_MESSAGE_FREE);
	for (int i = 0; i < 7*OCPP_DEFAULT_TX_RETRIES; i++) {
		mock().expectOneCall("ocpp_send").andReturnValue(0);
		mock().expectOneCall("ocpp_recv").ignoreOtherParameters().andReturnValue(-ENOMSG);
		step(interval*i);
		check_tx(OCPP_MSG_ROLE_CALL, OCPP_MSG_DATA_TRANSFER);
	}

	mock().expectOneCall("on_ocpp_event").withParameter("event_type", OCPP_EVENT_MESSAGE_FREE);
	mock().expectOneCall("ocpp_send").andReturnValue(0);
	mock().expectOneCall("ocpp_recv").ignoreOtherParameters().andReturnValue(-ENOMSG);
	step(interval*OCPP_DEFAULT_TX_RETRIES*7);
	check_tx(OCPP_MSG_ROLE_CALL, OCPP_MSG_START_TRANSACTION);

	mock().expectOneCall("ocpp_send").andReturnValue(0);
	mock().expectOneCall("ocpp_recv").ignoreOtherParameters().andReturnValue(-ENOMSG);
	step(interval*OCPP_DEFAULT_TX_RETRIES*8);
	check_tx(OCPP_MSG_ROLE_CALL, OCPP_MSG_START_TRANSACTION);
}

TEST(Core, ShouldReturnNOMEM_WhenQueueIsFullWithTransactionRelatedMessages) {
	struct ocpp_DataTransfer data;
	struct ocpp_StartTransaction start;
	for (int i = 0; i < 8; i++) {
		LONGS_EQUAL(0, ocpp_push_request(OCPP_MSG_DATA_TRANSFER, &data, sizeof(data), false));
	}
	mock().expectNCalls(8, "on_ocpp_event").withParameter("event_type", OCPP_EVENT_MESSAGE_FREE);
	for (int i = 0; i < 8; i++) {
		LONGS_EQUAL(0, ocpp_push_request(OCPP_MSG_START_TRANSACTION, &start, sizeof(start), true));
	}
	LONGS_EQUAL(-ENOMEM, ocpp_push_request(OCPP_MSG_START_TRANSACTION, &start, sizeof(start), true));
}

TEST(Core, ShouldDropTransactionRelatedMessages_WhenServerReponsesWithErrorMoreThanMaxAttemptsConfigured) {
}

TEST(Core, ShouldSendTransactionRelatedmessagesIndefinitely_WhenTransportErrors) {
}

TEST(Core, t) {
	ocpp_send_bootnotification(&(const struct ocpp_BootNotification) {
		.chargePointModel = "Model",
		.chargePointVendor = "Vendor",
	});

	mock().expectOneCall("ocpp_send").andReturnValue(0);
	mock().expectOneCall("ocpp_recv").ignoreOtherParameters().andReturnValue(-ENOMSG);
	step(0);
	check_tx(OCPP_MSG_ROLE_CALL, OCPP_MSG_BOOTNOTIFICATION);
}
