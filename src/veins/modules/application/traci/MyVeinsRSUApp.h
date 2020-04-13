#pragma once

#include "veins/veins.h"
#include <tr1/unordered_map>
#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"
#include "veins/modules/application/traci/infoMsg_m.h"
#include "veins/modules/application/traci/reportMsg_m.h"
#include "veins/modules/application/traci/myClasses.h"

namespace veins {

class VEINS_API MyVeinsRSUApp: public DemoBaseApplLayer {
public:
	void initialize(int stage) override;
	void finish() override;
	enum MyApplMessageKinds {
		SEND_INFOMSG_EVT,
		//SEND_REPORTMSG_EVT,
		INFO_MSG,
		REPORT_MSG
	};
protected:
	void onWSM(BaseFrame1609_4 *wsm) override;
	//void onWSA(DemoServiceAdvertisment *wsa) override;

	void handleSelfMsg(cMessage *msg) override;
	void handlePositionUpdate(cObject *obj) override;
	void initVehicle(int vehId);
	void deleteBasket(reportsBasket *basket);
	void genarateSecondaryScores();
	float calcSecondaryScore(int vehId);
	reportsBasket *currentBasket;
	reportsBasket *stagedBasket;
	int_2_intSet archivedScope; //  { senderId : { msgId : val } }

};

}
