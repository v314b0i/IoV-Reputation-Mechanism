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
	return ((rand() % 1000) < (accuracy * 1000)) ? val : !val;
}
float MyVeinsApp::scoreCalculator(float old, int trueCount, int count) {
	float fn = exp(-0.02 * count); // y= e^(-0.006*x)    y goes from 1 to 0 for x from 0 to infinity
	return fn * old + (1 - fn) * ((float) trueCount / (float) count);
}

void MyVeinsApp::initialize(int stage) {
	DemoBaseApplLayer::initialize(stage);
	if (stage == 0) {
		EV << "Initializing " << par("appName").stringValue() << std::endl;
		sent = 0;
		sentCorrect = 0;
		recMsg = 0;
		recRprt = 0;
		sentRprt = 0;
		messageInterval = par("messageInterval");
		messageIntervalVarianceLimit = par("messageIntervalVarianceLimit");
		reportGenTime = par("reportGenTime");
		reportGenTimeVarianceLimit = par("reportGenTimeVarianceLimit");
		messageIntervalVarianceLimit = SimTime(
				messageIntervalVarianceLimit.inUnit(SIMTIME_S), SIMTIME_MS);
		reportGenTimeVarianceLimit = SimTime(
				reportGenTimeVarianceLimit.inUnit(SIMTIME_S), SIMTIME_MS);
		evaluatingAccuracy =
				(float) par("evaluatingAccuracyPercentage").intValue()
						/ (float) 100;
		sendingAccuracy = setSendingAccuracy();
		sendMsgEvt = new cMessage("Send Message Event", SEND_INFOMSG_EVT);
		sentVector.setName("sentVector");
		sentCorrectVector.setName("sentCorrectVector");
		sentReportsVector.setName("sentReportsVector");
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
	recordScalar("#sentReports", sentRprt);
	int recievedMessages = 0;
	int recievedReports = 0;
	for (auto x : vehicles) {
		recievedMessages += x.second->msgCount;
		recievedReports += x.second->reportedCount;
	}
	recordScalar("recievedMessagesSum", recievedMessages);
	recordScalar("recievedReportsSum", recievedReports - recievedMessages);
	recordScalar("#recievedMessages", recMsg);
	recordScalar("#recievedReports", recRprt);

	recordScalar("mySendingAccuracy", sendingAccuracy);
	recordScalar("myFinalRealAccuracy", (float) sentCorrect / (float) sent); //if set sending accuracy is fuzzy the actual accuracy will wary while #sent is not large.

	char name[25];
	for (auto it = repScoreStats.begin(); it != repScoreStats.end(); ++it) {
		sprintf(name, "RepScoreStats-%d", (it->first - 15) / 6);
		it->second->recordAs(name);
		delete it->second;
	}
	for (auto it = repScoreVector.begin(); it != repScoreVector.end(); ++it)
		delete it->second;
	for (auto it = vehicles.begin(); it != vehicles.end(); ++it) {
		for (auto it2 = it->second->messages.begin();
				it2 != it->second->messages.end(); ++it2)
			delete it2->second;
		delete it->second;
	}
	for (auto it = reportedVector.begin(); it != reportedVector.end(); ++it)
		delete it->second;
	for (auto it = reportedTrueVector.begin(); it != reportedTrueVector.end();
			++it)
		delete it->second;
	for (auto it = reportComparisonVector.begin();
			it != reportComparisonVector.end(); ++it)
		delete it->second;
	for (auto it = msgVector.begin(); it != msgVector.end(); ++it)
		delete it->second;

	DemoBaseApplLayer::finish();
}

void MyVeinsApp::onWSM(BaseFrame1609_4 *frame) { //TODO restructure to remove redundant code
	//TODO deal with reports on self, or maybe not
	if (infoMsg *wsm = dynamic_cast<infoMsg*>(frame)) { //TODO
		recMsg++;
		EV_WARN << INFO_MSG;
		int senderId = wsm->getSenderAddress();
		int msgId = wsm->getMsgId();
		if (vehicles.find(senderId) == vehicles.end()) //if neither a message from this sender not a report on this sender has been recieved before
			initVehicle(senderId);
		vehStats &veh = *(vehicles[senderId]);
		if (veh.messages.find(msgId) == veh.messages.end()) //if neither a report of this message has been received earlier nor this message
			veh.messages[msgId] = new reporterId_2_val;
		reporterId_2_val &reports = *(veh.messages[msgId]);
		if (reports.find(myId) == reports.end()) { //if this message has been received for the first time by SELF..
			bool msgVal = inaccurateBoolCheck(wsm->getCorrect(),
					evaluatingAccuracy);
			reports[myId] = msgVal;
			//can instead here just do wsm = rep at the end and let the report handling happen in the next if block itself
			veh.msgCount++;
			veh.reportedCount++;
			if (msgVal) {
				veh.trueMsgCount++;
				veh.reportedTrueCount++;
			}
			veh.rep = scoreCalculator(veh.repOrignal, veh.reportedTrueCount,
					veh.reportedCount);

			repScoreVector[senderId]->record(veh.rep); //for simulation stats collection only
			repScoreStats[senderId]->collect(veh.rep); //for simulation stats collection only
			reportedVector[senderId]->record(veh.reportedCount);
			reportedTrueVector[senderId]->record(veh.reportedTrueCount);
			msgVector[senderId]->record(veh.msgCount);

			reportMsg *rep = new reportMsg();
			populateWSM(rep);
			rep->setReporterAddress(myId);
			rep->setReporteeAddress(senderId);
			rep->setReportedMsgId(msgId);
			rep->setFoundValid(msgVal);
			rep->setKind(REPORT_MSG);
			srand((int) simTime().raw() + myId);
			simtime_t variance = reportGenTimeVarianceLimit
					* ((float) (rand() % 1000) / (float) 500 - 1);
			scheduleAt(simTime() + reportGenTime + variance, rep);
		}
	} else if (reportMsg *wsm = dynamic_cast<reportMsg*>(frame)) {
		recRprt++;
		EV_WARN << REPORT_MSG;
		int reporteeId = wsm->getReporteeAddress();
		int reporterId = wsm->getReporterAddress();
		int msgId = wsm->getReportedMsgId();
		bool foundValid = wsm->getFoundValid();
		if (vehicles.find(reporterId) == vehicles.end())
			initVehicle(reporterId);
		vehStats &reporter = *(vehicles[reporterId]);
		if (vehicles.find(reporteeId) == vehicles.end())
			initVehicle(reporteeId);
		vehStats &reportee = *(vehicles[reporteeId]);

		if (reportee.messages.find(msgId) == reportee.messages.end()) //if neither a report of this message has been received earlier nor this message
			reportee.messages[msgId] = new reporterId_2_val;
		reporterId_2_val &reports = *(reportee.messages[msgId]);
		if (reports.find(reporterId) == reports.end()) { //if this report has not been recieved before
				//sendDown(wsm->dup());
			if (reports.find(myId) == reports.end()) { // if the message being reported HAS NOT been recieved by self (we update the reporteE's score as per report)
				reports[reporterId] = foundValid;
				reportee.reportedCount++;
				if (foundValid)
					reportee.reportedTrueCount++;
				reportee.rep = scoreCalculator(reportee.repOrignal,
						reportee.reportedTrueCount, reportee.reportedCount);

				repScoreVector[reporteeId]->record(reportee.rep); //for simulation stats collection only
				repScoreStats[reporteeId]->collect(reportee.rep); //for simulation stats collection only
				reportedVector[reporteeId]->record(reportee.reportedCount);
				reportedTrueVector[reporteeId]->record(
						reportee.reportedTrueCount);

			} else { // if the message being reported HAS been recieved by self (we update the reporteR's score as per its consistency with self's report)
				/*reporter.reportedCount++;
				 reporter.reportComparisonCount++;
				 if (reports[reporterId] == reports[myId]) {
				 reporter.reportedTrueCount++;
				 reporter.reportComparisonTrueCount++;
				 }
				 reporter.rep = scoreCalculator(reporter.repOrignal,
				 reporter.reportedTrueCount, reporter.reportedCount);
				 repScoreVector[reporterId]->record(reporter.rep); //for simulation stats collection only
				 repScoreStats[reporterId]->collect(reporter.rep); //for simulation stats collection only
				 reportedVector[reporterId]->record(reporter.reportedCount);
				 reportedTrueVector[reporterId]->record(
				 reporter.reportedTrueCount);
				 reportComparisonVector[reporterId]->record(
				 reporter.reportComparisonCount);*/
			}

		}
	}
}
void MyVeinsApp::initVehicle(int id) {
	vehicles[id] = new vehStats();
	//vehicles[id]->messages = new reporterId_2_val;
	//reportsArchive[id] = new msgId_2_reporterId2val();

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

/*void MyVeinsApp::onWSA(DemoServiceAdvertisment *wsa) {
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
}*/

void MyVeinsApp::handleSelfMsg(cMessage *msg) {
	if (msg->getKind() == SEND_INFOMSG_EVT) {
		infoMsg *wsm = new infoMsg();
		populateWSM(wsm);
		wsm->setSenderAddress(myId);
		wsm->setMsgId(sent++);
		wsm->setCorrect(inaccurateBoolCheck(true, sendingAccuracy));
		wsm->setKind(INFO_MSG);
		if (wsm->getCorrect())
			sentCorrectVector.record(++sentCorrect);
		sentVector.record(sent);
		sendDown(wsm);
		srand((int) simTime().raw() + myId);
		simtime_t variance = messageIntervalVarianceLimit
				* (((float) (rand() % 1000) / (float) 500) - 1);
		scheduleAt(simTime() + messageInterval + variance, sendMsgEvt);
	} else if (infoMsg *wsm = dynamic_cast<infoMsg*>(msg))
		sendDown(wsm);
	else if (reportMsg *wsm = dynamic_cast<reportMsg*>(msg)) {
		sendDown(wsm);
		sentReportsVector.record(++sentRprt);
	} else
		DemoBaseApplLayer::handleSelfMsg(msg);
}

void MyVeinsApp::handlePositionUpdate(cObject *obj) {
	DemoBaseApplLayer::handlePositionUpdate(obj);
}

