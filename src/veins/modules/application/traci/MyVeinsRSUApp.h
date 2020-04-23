#pragma once

#include "veins/veins.h"
#include <tr1/unordered_map>
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/infoMsg_m.h"
#include "veins/modules/application/traci/reportMsg_m.h"
#include "veins/modules/application/traci/myClasses.h"
#include "veins/modules/application/traci/myMiscFunctions.h"

namespace veins {

class VEINS_API MyVeinsRSUApp: public DemoBaseApplLayer {
public:
	void initialize(int stage) override;
	void finish() override;
protected:
	void handleSelfMsg(cMessage *msg) override;
	void handlePositionUpdate(cObject *obj) override;
	void onWSM(BaseFrame1609_4 *wsm) override;
	void ingestReport(int reporterId, int reporteeId, int msgId, bool foundValid);
	void addVehicletoCurrentBasket(int vehId);
	void initVehicleStats(int vehId);
	void stageShift();
	void deleteBasket(reportsBasket *basket);
	int_2_float genarateSecondaryScores(reportsBasket *basket);
	void populateBlacklistedReportersList(int_2_float secondaryScores, intSet &blacklist);
	int_2_float genaratePrimaryScores_ReportsBased(reportsBasket *basket, int_2_float secondaryScores,
			intSet blacklist);
	int_2_float genaratePrimaryScores_MessagesBased_MajorityAbsolutist(reportsBasket *basket,
			int_2_float secondaryScores, intSet blacklist);
	int_2_float genaratePrimaryScores_MessagesBased(reportsBasket *basket, int_2_float secondaryScores,
			intSet blacklist);
	cMessage *stageShiftEvt;
	reportsBasket *currentBasket;
	reportsBasket *stagedBasket;
	int_2_intSet archivedScope; //  { senderId : { msgId : val } }
	simtime_t stageShiftInterval;

	//FOR STATS
	int stageCounter;
	int recievedReports;
	cOutVector recievedReportsVector;
	intSet statsInitialisedVehicles;
	std::tr1::unordered_map<int, cOutVector*> secondaryScoreVector;
	std::tr1::unordered_map<int, cOutVector*> primaryScore_ReportsBasedVector;
	std::tr1::unordered_map<int, cOutVector*> primaryScore_MessagesBased_MajorityAbsolutistVector;
	std::tr1::unordered_map<int, cOutVector*> primaryScore_MessagesBasedVector;
	std::tr1::unordered_map<int, cOutVector*> primaryScore_RAW_Vector;
	cOutVector secondaryScoreThreshhold;

	cOutVector vehiclesInArchivedScope;
	cOutVector messagesInArchivedScope;

	cOutVector vehiclesInStagedScope;
	cOutVector messagesInStagedScope;
	cOutVector reportsInStagedBasket;
	cOutVector reportersInStagedBasket;
	cOutVector blacklistedReportersInStagedBasket;

	cOutVector vehiclesInCurrentScope;
	cOutVector messagesInCurrentScope;
	cOutVector reportsInCurrentBasket;
	cOutVector reportersInCurrentBasket;

	int_2_int reportsOn;
	int_2_int reportsFrom;
};

}
