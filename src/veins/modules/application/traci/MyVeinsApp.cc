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
#include <string.h>
using namespace veins;

Define_Module(veins::MyVeinsApp);

bool MyVeinsApp::inaccurateBoolCheck(bool val, float accuracy) {
	srand((int) simTime().raw() + myId);
	if ((rand() % 1000) < (accuracy * 1000))
		return val;
	return val ? false : true;
}
float MyVeinsApp::scoreCalculator(float old, int trueCount, int count) {
	float fn = exp(-0.006 * count); // y= e^(-0.006*x)    y goes from 1 to 0 for x from 0 to infinity
	return fn * old + (1 - fn) * ((float) trueCount / (float) count);
}

void MyVeinsApp::initialize(int stage) {
	DemoBaseApplLayer::initialize(stage);
	if (stage == 0) {
		EV << "Initializing " << par("appName").stringValue() << std::endl;
		currentSubscribedServiceId = -1;
		sent = 0;
		sentCorrect = 0;
		messageInterval = par("messageInterval");
		messageIntervalVarianceLimit = par("messageIntervalVarianceLimit");
		messageIntervalVarianceLimit = SimTime(
				messageIntervalVarianceLimit.inUnit(SIMTIME_S), SIMTIME_MS);
		evaluatingAccuracy =
				(float) par("evaluatingAccuracyPercentage").intValue()
						/ (float) 100;
		sendingAccuracy = setSendingAccuracy();
		sendMsgEvt = new cMessage("Send Message Event", SEND_INFOMSG_EVT);
		//sendReportEvt = new cMessage("Send Report Event", SEND_REPORTMSG_EVT);
		//sendReportEvt->
		//todo create enum of message kinds in .h file such that SEND_MSG_EVT is mapped to 0, see demobaseapp for reference.

		sentVector.setName("sentVector");
		sentCorrectVector.setName("sentCorrectVector");

	} else if (stage == 1) {
		//manually setting nodes 1,3 as bad and 0,2 as good.
		if ((int) myId == 21 || (int) myId == 33) {
			sendingAccuracy =
					(float) par("badSendingAccuracyPercentage").intValue()
							/ (float) 100;
		} else if ((int) myId == 15 || (int) myId == 27) {
			sendingAccuracy =
					(float) par("goodSendingAccuracyPercentage").intValue()
							/ (float) 100;
		}
		srand(myId);
		simtime_t variance = messageIntervalVarianceLimit
				* (((float) (rand() % 1000) / (float) 500) - 1); //variance=(-limit to +limit)uniformly
		scheduleAt(simTime() + messageInterval + variance, sendMsgEvt);
	}
}

float MyVeinsApp::setSendingAccuracy() {
	srand(myId + (int) simTime().raw());
	int a = (rand() % 1000);
	return (float) (
			a < (par("percentageWithBadSendingAccuracy").intValue() * 10) ?
					par("badSendingAccuracyPercentage").intValue() :
					par("goodSendingAccuracyPercentage").intValue())
			/ (float) 100;
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
	char name[25];
	for (auto it = repScoreStats.begin(); it != repScoreStats.end(); ++it) {
		sprintf(name, "RepScoreStats-%d", (it->first - 15) / 6);
		it->second->recordAs(name);
		delete it->second;
	}
	for (auto it = repScoreVector.begin(); it != repScoreVector.end(); ++it)
		delete it->second;
	for (auto it = vehicles.begin(); it != vehicles.end(); ++it)
		delete it->second;
	for (auto it = reportsArchive.begin(); it != reportsArchive.end(); ++it)
		delete it->second;
	for (auto it = reportedVector.begin(); it != reportedVector.end(); ++it)
			delete it->second;
	for (auto it = reportedTrueVector.begin(); it != reportedTrueVector.end(); ++it)
			delete it->second;
	for (auto it = reportComparisonVector.begin(); it != reportComparisonVector.end(); ++it)
			delete it->second;
	for (auto it = msgVector.begin(); it != msgVector.end(); ++it)
			delete it->second;

	DemoBaseApplLayer::finish();
}

void MyVeinsApp::onWSM(BaseFrame1609_4 *frame) { //TODO restructure to remove redundant code
	if (infoMsg *wsm = dynamic_cast<infoMsg*>(frame)) { //TODO
		EV_WARN << INFO_MSG;
		int senderId = wsm->getSenderAddress();
		int msgId = wsm->getMsgId();
		std::pair<int, int> senderId_msgId = std::make_pair(senderId, msgId);
		if (vehicles.find(senderId) == vehicles.end()) //if neither a message from this sender not a report on this sender has been recieved before
			initVehicle(senderId);
		vehStats *veh = vehicles[senderId];
		if (reportsArchive.find(senderId_msgId) == reportsArchive.end()) //if neither a report of this message has been received earlier nor this message
			reportsArchive[senderId_msgId] = new int2boolmap;
		int2boolmap *reports = reportsArchive[senderId_msgId];
		if (reports->find(myId) == reports->end()) { //if this message has been received for the first time by SELF..
			bool msgVal = inaccurateBoolCheck(wsm->getCorrect(),
					evaluatingAccuracy);
			(*reports)[myId] = msgVal;
			//can instead create a fn "handleReport(reporterID,reporteeID,msgID,val)" to add entry to reportsArchive and also update rep,
			//same fn could be called from reportmsg handling block.
			veh->msgCount++;
			veh->reportedCount++;
			if (msgVal) {
				veh->trueMsgCount++;
				veh->reportedTrueCount++;
			}
			veh->rep = scoreCalculator(veh->repOrignal, veh->reportedTrueCount,
					veh->reportedCount);

			repScoreVector[senderId]->record(veh->rep); //for simulation stats collection only
			repScoreStats[senderId]->collect(veh->rep); //for simulation stats collection only
			reportedVector[senderId]->record(veh->reportedCount);
			reportedTrueVector[senderId]->record(veh->reportedTrueCount);
			msgVector[senderId]->record(veh->msgCount);
			reportMsg *rep = new reportMsg();
			populateWSM(rep);
			rep->setReporterAddress(myId);
			rep->setReporteeAddress(senderId);
			rep->setReportedMsgId(msgId);
			rep->setFoundValid(msgVal);
			rep->setKind(REPORT_MSG);
			if (dataOnSch) {
				startService(Channel::sch2, 1337, "My test Service");
				scheduleAt(
						computeAsynchronousSendingTime(1, ChannelType::service),
						rep);
			} else
				sendDown(rep);
		}
	} else if (reportMsg *wsm = dynamic_cast<reportMsg*>(frame)) {
		EV_WARN << REPORT_MSG;
		int reporteeId = wsm->getReporteeAddress();
		int reporterId = wsm->getReporterAddress();
		int msgId = wsm->getReportedMsgId();
		bool foundValid = wsm->getFoundValid();
		std::pair<int, int> reporteeId_msgId = std::make_pair(reporteeId,
				msgId);
		if (reportsArchive.find(reporteeId_msgId) == reportsArchive.end()) //if neither a report of this message has been received earlier nor this message
			reportsArchive[reporteeId_msgId] = new int2boolmap;	//TODO add delay here or timer via self msg so that its more likely to recieve a msg first before receiveing a report on it.
		int2boolmap *reports = reportsArchive[reporteeId_msgId];
		if (reports->find(reporterId) == reports->end()) { //if this report has not been recieved before
			//sendDown(wsm);
			if (reports->find(myId) == reports->end()) { // if the message being reported HAS NOT been recieved by self (we update the reporteE's score as per report)
				(*reports)[reporterId] = foundValid;
				if (vehicles.find(reporteeId) == vehicles.end()) //if neither a message from this sender not a report on this sender has been recieved before
					initVehicle(reporteeId);
				vehStats *veh = vehicles[reporteeId];
				veh->reportedCount++;
				if (foundValid)
					veh->reportedTrueCount++;
				veh->rep = scoreCalculator(veh->repOrignal,
						veh->reportedTrueCount, veh->reportedCount);

				repScoreVector[reporteeId]->record(veh->rep); //for simulation stats collection only
				repScoreStats[reporteeId]->collect(veh->rep); //for simulation stats collection only
				reportedVector[reporteeId]->record(veh->reportedCount);
				reportedTrueVector[reporteeId]->record(veh->reportedTrueCount);

			} else { // if the message being reported HAS been recieved by self (we update the reporteR's score as per its consistency with self's report)
				if (vehicles.find(reporterId) == vehicles.end()) //if neither a message from this reportee not a report on this reportee has been recieved before
					initVehicle(reporterId);
				vehStats *veh = vehicles[reporterId];
				veh->reportedCount++;
				veh->reportComparisonCount++;
				if ((*reports)[reporterId] == (*reports)[myId]) {
					veh->reportedTrueCount++;
					veh->reportComparisonTrueCount++;
				}
				veh->rep = scoreCalculator(veh->repOrignal,
						veh->reportedTrueCount, veh->reportedCount);
				repScoreVector[reporterId]->record(veh->rep); //for simulation stats collection only
				repScoreStats[reporterId]->collect(veh->rep); //for simulation stats collection only
				reportedVector[reporterId]->record(veh->reportedCount);
				reportedTrueVector[reporterId]->record(veh->reportedTrueCount);
				reportComparisonVector[reporterId]->record(
						veh->reportComparisonCount);
			}

		}
	}
}
void MyVeinsApp::initVehicle(int id) {
	vehicles[id] = new vehStats();
	repScoreStats[id] = new cHistogram; //for simulation stats collection only
	repScoreVector[id] = new cOutVector; //for simulation stats collection only
	reportedVector[id] = new cOutVector;
	reportedTrueVector[id] = new cOutVector;
	reportComparisonVector[id] = new cOutVector;
	msgVector[id] = new cOutVector;
	char name[40];
	sprintf(name, "RepScoreVector-%d", (id - 15) / 6); // node i's "id" is i*6+15
	repScoreVector[id]->setName(name); //for simulation stats collection only
	sprintf(name, "reportedVector-%d", (id - 15) / 6);
	reportedVector[id]->setName(name);
	sprintf(name, "reportedTrueVector-%d", (id - 15) / 6);
	reportedTrueVector[id]->setName(name);
	sprintf(name, "reportComparisonVector-%d", (id - 15) / 6);
	reportComparisonVector[id]->setName(name);
	sprintf(name, "msgVector-%d", (id - 15) / 6);
	msgVector[id]->setName(name);
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
	if (msg->getKind() == SEND_INFOMSG_EVT) {
		infoMsg *wsm = new infoMsg();
		populateWSM(wsm);
		wsm->setSenderAddress(myId);
		wsm->setMsgId(sent++);
		wsm->setCorrect(inaccurateBoolCheck(true, sendingAccuracy));
		wsm->setKind(INFO_MSG);
		//todo set msg kind

		if (wsm->getCorrect())
			sentCorrectVector.record(++sentCorrect);
		sentVector.record(sent);

		if (dataOnSch) {
			startService(Channel::sch2, 1337, "My test Service");
			scheduleAt(computeAsynchronousSendingTime(1, ChannelType::service),
					wsm);
		} else
			sendDown(wsm);

		srand((int) simTime().raw() + myId);
		simtime_t variance = messageIntervalVarianceLimit
				* ((float) (rand() % 1000) / (float) 500 - 1);
		scheduleAt(simTime() + messageInterval + variance, sendMsgEvt);

	} else if (infoMsg *wsm = dynamic_cast<infoMsg*>(msg))
		sendDown(wsm);
	else if (reportMsg *wsm = dynamic_cast<reportMsg*>(msg))
		sendDown(wsm);
	else
		DemoBaseApplLayer::handleSelfMsg(msg);
}

void MyVeinsApp::handlePositionUpdate(cObject *obj) {
	DemoBaseApplLayer::handlePositionUpdate(obj);
}

