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

void MyVeinsApp::initialize(int stage) {
	DemoBaseApplLayer::initialize(stage);
	if (stage == 0) {
		EV << "Initializing " << par("appName").stringValue() << std::endl;
		currentSubscribedServiceId = -1;
		sent = 0;
		sentCorrect=0;
		setSendingAccuracy();
	} else if (stage == 1) {
	}
}
void MyVeinsApp::setSendingAccuracy(int stage) {
	srand(myId);
	sendingAccuracy=(rand()%1000<100)?0.0:1.0;      //randomly set accuracy of 10% vehicles to 0 and remaining to 1.
	//can alternatively make a bool val called "isbad" and #define badAccuracy and goodAccuracy and pass these in inaccurateboolcheck according to isbad value.
}




void MyVeinsApp::finish() {
	recordScalar("#sent", sent);
	int recieved = 0;
	for (auto x : counts)
		recieved += x.second.first;
	recordScalar("#recieved", recieved);
	recordScalar("mySendingAccuracy", sendingAccuracy);
	//accuracy can be made fuzzy for good and bad in other experiments, there actual accuracy will wary from set accuracy especially if 'sent' is not large.
	recordScalar("myFinalRealAccuracy", (float)sentCorrect/(float)sent)
	DemoBaseApplLayer::finish();
}
void MyVeinsApp::onWSM(BaseFrame1609_4 *frame) {
	try {
		infoMsg *wsm = check_and_cast<infoMsg*>(frame);
		try {
			counts[wsm->getSenderAddress()].first++;
		} catch (...) {
			counts[wsm->getSenderAddress()] = std::make_pair((int) 1,
					(double) 0);
		}
		EV << "\n------------------------------------------\n";
		EV << "counts for" << myId << " are:\n";
		for (auto x : counts)
			EV << x.first << "-sent-" << x.second.first << "\n";
		EV << "------------------------------------------\n";
	} catch (...) {
		try {
			reportMsg *wsm = check_and_cast<reportMsg*>(frame);
			EV << "TODO";
		} catch (...) {
			EV << "Unknown type WSS Recieved";
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
		wsm->setCorrect(inaccurateBoolCheck(true,sendingAccuracy));
		if(wsm->getCorrect()) sentCorrect++;
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

bool MyVeinsApp::inaccurateBoolCheck(bool val, float accuracy = 0.9) {
	srand((int) simTime().raw() + myId);
	if ((rand() % 1000) < (accuracy * 1000))
		return val;
	return val ? false : true;
}
