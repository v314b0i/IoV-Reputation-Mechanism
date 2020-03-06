//
// Copyright (C) 2016 David Eckhoff <david.eckhoff@fau.de>
//
// Documentation for these modules is at http://veins.car2x.org/
//
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "veins/modules/application/traci/MyVeinsApp.h"
#include "veins/modules/application/traci/infoMsg_m.h"
#include "veins/modules/application/traci/reportMsg_m.h"
#include <stdlib.h>
using namespace veins;

Define_Module(veins::MyVeinsApp);

bool MyVeinsApp::inaccurateBoolCheck(bool val, float accuracy) {
	srand((int) simTime().raw() + myId);
	if ((rand() % 1000) < (accuracy * 1000))
		return val;
	return val ? false : true;
}
float MyVeinsApp::scoreCalculator(float old, int trueCount, int count) {
	float fn = exp(-0.006 * count);     // y= e^(-0.006*x)
	return fn * old + (1 - fn) * ((float) trueCount / (float) count);
}

void MyVeinsApp::initialize(int stage) {
	DemoBaseApplLayer::initialize(stage);
	if (stage == 0) {
		EV << "Initializing " << par("appName").stringValue() << std::endl;
		currentSubscribedServiceId = -1;
		sent = 0;
		sentCorrect = 0;
		evaluatingAccuracy = 1;
		setSendingAccuracy();
	} else if (stage == 1) {
	}
}
void MyVeinsApp::setSendingAccuracy() {
	srand(myId);
	sendingAccuracy = (rand() % 1000 < 100) ? 0.0 : 1.0; //randomly set accuracy of 10% vehicles to 0 and remaining to 1.
	//can alternatively make a bool val called "isbad" and #define badAccuracy and goodAccuracy and pass these in inaccurateboolcheck according to isbad value.
}

void MyVeinsApp::finish() {
	recordScalar("#sent", sent);
	int recieved = 0;
	for (auto x : vehicles)
		recieved += x.second->msgCount;
	recordScalar("#recieved", recieved);
	recordScalar("mySendingAccuracy", sendingAccuracy);
	//accuracy can be made fuzzy for good and bad in other experiments, there actual accuracy will wary from set accuracy especially if 'sent' is not large.
	recordScalar("myFinalRealAccuracy", (float) sentCorrect / (float) sent);
	DemoBaseApplLayer::finish();
}
void MyVeinsApp::onWSM(BaseFrame1609_4 *frame) {
	if (infoMsg *wsm = dynamic_cast<infoMsg*>(frame)) {
		int senderId = wsm->getSenderAddress();
		int msgId = wsm->getMsgId();
		std::pair<int, int> senderId_msgId = std::make_pair(senderId, msgId);
		if (vehicles.find(senderId) == vehicles.end()) //if a message has been received from this sender for the first time...
			vehicles[senderId] = new vehStats();
		vehStats *veh = vehicles[senderId];
		if (reportsArchive.find(senderId_msgId) == reportsArchive.end()) //if neither a report of this message has been received earlier nor this message
			reportsArchive[senderId_msgId] = new int2boolmap;
		int2boolmap *reports = reportsArchive[senderId_msgId];
		if (reports->find(myId) == reports->end()) { //if this message has been received for the first time by SELF..
			bool msgVal = inaccurateBoolCheck(wsm->getCorrect(),
					evaluatingAccuracy);
			(*reports)[myId] = msgVal;
			//can instead create a fn "handleReport(reporterID,reporteeID,msgID,val)" to add entry to reportsArchive and also update rep,
			//same fn could be called from reportmsg handing block.
			veh->msgCount++;
			veh->reportedCount++;
			if (msgVal) {
				veh->trueMsgCount++;
				veh->reportedTrueCount++;
			}
			veh->rep = scoreCalculator(veh->repOrignal, veh->reportedTrueCount,
					veh->reportedCount);
			EV << "\n------------------------------------------\n";
			EV << "myId=" << myId << "\nSenderId=" << senderId << "\tcount="
						<< veh->reportedCount << "\ttrue="
						<< veh->reportedTrueCount << "\tscore" << veh->rep;
		}
	}
}

void MyVeinsApp::onWSA(DemoServiceAdvertisment *wsa) {
	EV << "onWSAinvoked: DemoServiceAdvertisment";
	if (currentSubscribedServiceId == -1) {
		mac->changeServiceChannel(
				static_cast<Channel>(wsa->getTargetChannel()));
		currentSubscribedServiceId = wsa->getPsid();
		if (currentOfferedServiceId != wsa->getPsid()) {
			stopService();
			startService(static_cast<Channel>(wsa->getTargetChannel()),
					wsa->getPsid(), "Mirrored Traffic Service");
		}
	}
}

void MyVeinsApp::handleSelfMsg(cMessage *msg) {
	if (infoMsg *wsm = dynamic_cast<infoMsg*>(msg)) {
		sendDown(wsm);
	} else {
		DemoBaseApplLayer::handleSelfMsg(msg);
	}
}

void MyVeinsApp::handlePositionUpdate(cObject *obj) {
	DemoBaseApplLayer::handlePositionUpdate(obj);
	srand((int) simTime().raw() + myId);
	if (rand() % 1000 < 50) {
		infoMsg *wsm = new infoMsg();
		populateWSM(wsm);
		wsm->setSenderAddress(myId);
		wsm->setMsgId(sent++);
		wsm->setCorrect(inaccurateBoolCheck(true, sendingAccuracy));
		if (wsm->getCorrect())
			sentCorrect++;
		if (dataOnSch) {
			startService(Channel::sch2, 1337, "My test Service");
			// started service and server advertising, schedule message to self to send later
			scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service),
					wsm);
		} else {
			// send right away on CCH, because channel switching is disabled
			sendDown(wsm);
		}
	}
// the vehicle has moved. Code that reacts to new positions goes here.
// member variables such as currentPosition and currentSpeed are updated in the parent class
}

