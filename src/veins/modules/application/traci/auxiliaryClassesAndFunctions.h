#ifndef MY_AUX_HEADER_FILE
#define MY_AUX_HEADER_FILE

#include <tr1/unordered_map>
#include <map>
#include <tr1/unordered_set>
#include <string>
#include <vector>
#include <math.h>
#include <algorithm>
#include <sstream>
#include <fstream>
//TODO create separate structs for stats collection counters, keep app functionality and ststs collection differentiated.
enum MyApplMessageKinds {
	SEND_INFOMSG_EVT, STAGE_SHIFT_EVT, INFO_MSG, REPORT_MSG
};
class recorder {
	std::string filename;
public:
	recorder(std::string fn) : filename(std::string("R/").append(fn)) {
		remove(filename.c_str());
	}
	recorder() {
		filename = std::string("R/").append("uninitialised.txt");
	}
	void setName(std::string fn) {
		filename = std::string("R/").append(fn);
		remove(filename.c_str());
	}
	template<typename T> void record(T val) {
		recordString(std::to_string(val));
	}
	void recordString(std::string str) {
		std::ofstream fout(filename.c_str(), std::ios::app);
		fout << str.append(",");
		fout.close();
	}
	void deletefile() {
		remove(filename.c_str());
	}
};
inline int id2num(int id, int node0id) {
	return (id - node0id) / 6;
}
inline int num2id(int id, int node0id) {
	return (id * 6) + node0id;
}
typedef std::tr1::unordered_map<int, bool> int_2_bool;
typedef std::tr1::unordered_map<int, float> int_2_float;
typedef std::tr1::unordered_map<int, int> int_2_int;
typedef std::tr1::unordered_set<int> intSet;
typedef std::tr1::unordered_map<int, int_2_float> int_2_int2float;
typedef std::tr1::unordered_map<int, int_2_bool*> int_2_int2bool;
typedef std::tr1::unordered_map<int, intSet*> int_2_intSet;
typedef std::tr1::unordered_map<int, std::vector<float>> int_2_floatVec;
/*
 template<typename containertype, typename keytype, typename valtype> containertype getMapValues(
 std::tr1::unordered_map<keytype, valtype> m);
 template<typename containertype, typename keytype, typename valtype> containertype getMapKeys(
 std::tr1::unordered_map<keytype, valtype> m);
 template<typename type> float vectorMedian(std::vector<type> vec);
 template<typename type> float vectorMean(std::vector<type> vec);
 template<typename type> float vectorMode(std::vector<type> vec);
 template<typename type> float standardDeviation(std::vector<type> v, float centre);
 template<typename type> float medianAbsoluteDeviation(std::vector<type> v, float centre);
 template<typename type> bool ismultimodal(std::vector<type> vec, float median);
 template<typename type> bool ismultimodal(std::vector<type> vec);
 intSet csvToIntSet(std::string csvstr);
 */
template<typename containertype, typename keytype, typename valtype> inline containertype getMapValues(
		std::tr1::unordered_map<keytype, valtype> m) {
	containertype x;
	for (auto i : m)
		x.insert(x.end(), i.second);
	return x;
}
template<typename containertype, typename keytype, typename valtype> inline containertype getMapKeys(
		std::tr1::unordered_map<keytype, valtype> m) {
	containertype x;
	for (auto i : m)
		x.insert(x.end(), i.first);
	return x;
}
template<typename type> inline float vectorMedian(std::vector<type> vec) {
	if (vec.size() == 0)
		return -1;
	sort(vec.begin(), vec.end());
	return vec.at((int) (vec.size() / 2));
}
template<typename type> inline float vectorMean(std::vector<type> vec) {
	if (vec.size() == 0)
		return -1;
	float acc = 0;
	for (auto i : vec)
		acc += i;
	return acc / (float) vec.size();
}
template<typename type> inline float vectorMode(std::vector<type> vec) {
	if (vec.size() == 0)
		return -1;
	//appx algo written by me. may need to be replaced if the isMultimodal func's temporary logic isn't replaced or this is used elsewhere
	int hist[10] = { 0 }, hist2[9] = { 0 }, hmax = 0; //hist[i]=bin(10*i% to 10*(i+1)%) and hist2[i]=bin(5+10*i% to 5+10*(i+1)%)
	float hmaxi = 0;
	for (auto it : vec) {
		if ((it) >= 0.95) {
			if (++hist[9] > hmax) {
				hmax = hist[9];
				hmaxi = 0.945;
			}
		} else if ((it) <= 0.05) {
			if (++hist[0] > hmax) {
				hmax = hist[0];
				hmaxi = 0.045;
			}
		} else {
			if (++hist[(int) ((it) * 10)] > hmax) {
				hmax = hist[(int) ((it) * 10)];
				hmaxi = 0.045 + ((float) ((int) ((it) * 10))) / 10;
			}
			if (++hist2[(int) ((it - 0.05) * 10)] > hmax) {
				hmax = hist2[(int) ((it - 0.05) * 10)];
				hmaxi = 0.095 + ((float) ((int) ((it - 0.05) * 10))) / 10;
			}
		}
	}
	return hmaxi;
}
template<typename type> inline float standardDeviation(std::vector<type> v, float centre) {
	float acc = 0;
	float diff;
	for (auto it : v) {
		diff = it - centre;
		acc += diff * diff;
	}
	return sqrt(acc / (float) v.size());
}
template<typename type> inline float medianAbsoluteDeviation(std::vector<type> v, float centre) {
	std::vector<float> vec;
	for (auto it : v)
		vec.insert(vec.end(), it > centre ? it - centre : centre - it);
	return vectorMedian(vec);
}

template<typename type> inline bool ismultimodal(std::vector<type> vec, float median) {
	//FIXME currently is an approximate testing function written by me. Should instead implement a proper test such as Hartigan's dip test
	//https://en.wikipedia.org/wiki/Multimodal_distribution#General_tests
	float mode = vectorMode(vec);
	float diffBetMedianAndMode = median > mode ? median - mode : mode - median;
	return diffBetMedianAndMode >= 0.20; // arbitrary cut-off. Use hartigans'd dip test and set cut off=0.1 on the p value.
	//should not consider the scores from nodes that had few total reports.
}
template<typename type> inline bool ismultimodal(std::vector<type> vec) {
	float median = vectorMedian(vec);
	return ismultimodal(vec, median);
}
inline intSet csvToIntSet(std::string csvstr) {
	std::tr1::unordered_set<int> set;
	std::string word;
	std::stringstream SS(csvstr.c_str());
	while (getline(SS, word, ','))
		set.insert(std::stoi(word));
	return set;
}
inline int_2_floatVec csv2DFloatValsParse(std::string csvstr, int numOVals) {
	int_2_floatVec parsedMap;
	std::string word;
	std::stringstream SS(csvstr.c_str());
	while (getline(SS, word, ',')) {
		int key = std::stoi(word);
		std::vector<float> vals;
		for (int i = 0; i < numOVals; ++i) {
			getline(SS, word, ',');
			vals.insert(vals.end(), std::stof(word));
		}
		parsedMap[key] = vals;
	}
	return parsedMap;
}
class vehStats {		//	{senderId : (rep, rep0, total#, true#, { msgId: { reporterId : val } }) }
public:
	float rep = 0.5;       		//current calculated rep of vehicle
	float repOrignal = 0.5;		//rep of vehicle from last server communication
	int reportedCount = 0; //number of reports received on this vehicle + number of reports by this vehicle that were compared with a report by self.
	int reportedTrueCount = 0; //number of reports received on this vehicle that stated it was truthful + number of reports by this vehicle that matched with a report by self.
	int_2_int2bool messages; // { msgId: { reporterId : val } }

	//--FOR STATS--
	int reportComparisonCount = 0; //number of times a report by it was compared by a report by self and result treated as a report on it
	int reportComparisonTrueCount = 0; //number of times a report by it was compared by a report by self and it was a match.
	int msgCount = 0;      		//messages received from veh
	int trueMsgCount = 0; //messages received from veh which were evaluated to true by self
};
typedef std::tr1::unordered_map<int, vehStats*> int2vehStats;

struct reportsGist {
	int reportedTrueCount = 0;
	int reportedCount = 0;
	int_2_bool messages;
};
typedef std::tr1::unordered_map<int, reportsGist*> int2reportsGist;

struct vehStatsEntityCentric {  // { vehId : (truefloat#, total#, { reporterId : (true#, total#, { msgId : val }) }) }
	int reportedTrueCount = 0;
	int reportedCount = 0;
	int2reportsGist reporters; // { reporterId : (true#, total#, { msgId : val }) }
};

typedef std::tr1::unordered_map<int, vehStatsEntityCentric*> int2vehStatsEntityCentric;

struct reportsBasket {
	int_2_intSet scope;
	int2vehStatsEntityCentric vehicles;
	inline bool contains(int veh, int msg) {
		return (scope.count(veh) && scope[veh]->count(msg));
	}
	inline bool contains(int veh) {
		return scope.count(veh);
	}
};
/*
 int uniqueReportersInBasket(reportsBasket basket);
 int reportsInBasket(reportsBasket basket);
 int vehiclesInScope(int_2_intSet scope);
 int messagesInScope(int_2_intSet scope);
 */
inline int uniqueReportersInBasket(reportsBasket basket) {
	intSet reporters;
	for (auto veh : basket.vehicles) {
		intSet vehsReporters = getMapKeys<intSet>(veh.second->reporters);
		reporters.insert(vehsReporters.begin(), vehsReporters.end());
	}
	return reporters.size();
}
inline int reportsInBasket(reportsBasket basket) {
	int reports = 0;
	for (auto veh : basket.vehicles) {
		for (auto reporter : veh.second->reporters) {
			reports += reporter.second->reportedCount;
		}
	}
	return reports;
}
inline int vehiclesInScope(int_2_intSet scope) {
	return scope.size();
}
inline int messagesInScope(int_2_intSet scope) {
	int count = 0;
	for (auto i : scope)
		count += i.second->size();
	return count;
}

struct msgVal {
	int id;
	float truthValue;
	msgVal(int i, float Val) : id(i), truthValue(Val) {
	}
};
template<typename type> inline type calculatePowersAscending(int start, int factor, int level) {
	type sizes;
	int size = start;
	for (int i = 0; i < level; i++) {
		sizes.insert(sizes.end(), size);
		size *= factor;
	}
	return sizes;
}
struct vehMsgHistory {
	//can use ordered int->float map for messages instead of vector<{int,float}> but insertion and deletion complexity would be log(n)
	//would have to use it if we allow for older messges to be added that were skipped earlier (highly exceptional cases) but that adds
	//more complexity to the functions, not required for simulation.
	int logLimit;
	int splitFactor;
	int splitLevel;
	std::vector<msgVal> messages;
	int_2_float splitTotals;
	intSet splitSizes;
	vehMsgHistory(int start, int factor, int level) { //for f=5,l=4 : sS=<{5,-1},{25,-1},{125,-1},{625,-1}> , lL=625.
		splitFactor = factor;
		splitLevel = level;
		splitSizes = calculatePowersAscending<intSet>(start, factor, level);
		for (int size : splitSizes) {
			splitTotals[size] = 0;
			logLimit = size;
		}
	}

	void insert(int_2_float messageValsMap) {
		std::vector<int> keys = getMapKeys<std::vector<int>>(messageValsMap);
		std::sort(keys.begin(), keys.end());
		for (auto key : keys) {
			msgVal M(key, messageValsMap[key]);
			insert(M);
		}
	}

	void insert(msgVal M) {
		messages.insert(messages.begin(), M);
		for (auto size : splitSizes) {
			splitTotals[size] += M.truthValue;
			if (messages.size() > size)
				splitTotals[size] -= messages.data()[size].truthValue;
		}
		if (messages.size() > logLimit)
			messages.pop_back();
	}
	int_2_float getSplitAvgs() {
		int_2_float splitAvgs;
		for (auto size : splitSizes)
			if (messages.size() >= size)
				splitAvgs[size] = splitTotals[size] / size;
		return splitAvgs;
	}
	float getOverallAvg() {
		return splitTotals[logLimit] / (float) messages.size();
	}
};
struct messageGistInDynVehMsgHistory {
	intSet trueReporters;
	intSet falseReporters;
	bool fixed;
	bool evaluated;
	float avg;
	messageGistInDynVehMsgHistory() {
		fixed = false;
		evaluated = false;
		avg = 0.5;
	}
};
struct vehMsgHistoryDynamic {
	bool stageMessages;
	int stagingThreshhold;
	std::tr1::unordered_map<int, messageGistInDynVehMsgHistory> msgBuffer;
	int messagesRecv;
	int reportsRecv;
	int logLimit;
	int splitFactor;
	int splitLevel;
	int lockedMaxId; //if the rsu has sent evaluations of messages upto this message Id then we ignore messages and reports on messages less then it.
	std::tr1::unordered_map<int, std::map<int, messageGistInDynVehMsgHistory, std::greater<int>>*> messages; //ordered map
	int_2_float splitTotals;
	std::vector<int> splitSizes;
	vehMsgHistoryDynamic(int start, int factor, int level, bool staging = false, int stageingTh = 5) { //for f=5,l=4 : sS=<{5,-1},{25,-1},{125,-1},{625,-1}> , lL=625.
		splitFactor = factor;
		splitLevel = level;
		messagesRecv = 0;
		reportsRecv = 0;
		lockedMaxId = -1;
		splitSizes = calculatePowersAscending<std::vector<int>>(start, factor, level);
		stageMessages = staging;
		stagingThreshhold = stageingTh;
		for (int size : splitSizes) {
			messages[size] = new std::map<int, messageGistInDynVehMsgHistory, std::greater<int>>;
			splitTotals[size] = 0;
			logLimit = size;
		}
	}
	void addReport2message(messageGistInDynVehMsgHistory &msg, int reporterId, bool val) {
		if (val)
			msg.trueReporters.insert(reporterId);
		else msg.falseReporters.insert(reporterId);
		float avg = (float) msg.trueReporters.size();
		avg /= avg + (float) msg.falseReporters.size();
		msg.avg = avg;
	}
	void insertReport(int reporterId, int msgId, bool val) {
		recorder rec("insertReportRecorder");
		rec.record(1);
		if (lockedMaxId >= msgId)
			return;
		if (stageMessages and msgBuffer.count(msgId)) {
			messageGistInDynVehMsgHistory &msg = msgBuffer[msgId];
			addReport2message(msgBuffer[msgId], reporterId, val);
			if ((msgBuffer[msgId].falseReporters.size() + msgBuffer[msgId].trueReporters.size()) >= stagingThreshhold) {
				insertMsgObj(msgBuffer[msgId], msgId);
				msgBuffer.erase(msgId);
			}
		} else {
			bool foundFlag = false;
			for (int size : splitSizes) {
				rec.recordString(std::string("\nlooping-iteration-").append(std::to_string(size)));
				if (messages[size]->count(msgId)) {
					rec.recordString(std::string("in if"));
					messageGistInDynVehMsgHistory &msg = (*messages[size])[msgId];
					splitTotals[size] -= msg.avg;
					addReport2message(msg, reporterId, val);
					splitTotals[size] += msg.avg;
					rec.record(3);
					foundFlag = true;
					break;
				}
			}
			if (!foundFlag) {
				rec.recordString(std::string("\nlooping over\n"));
				messageGistInDynVehMsgHistory newMsgGistObj;
				addReport2message(newMsgGistObj, reporterId, val);
				if (stageMessages)
					msgBuffer[msgId] = newMsgGistObj;
				else insertMsgObj(newMsgGistObj, msgId);
			}
		}
		int found = 0;
		if (msgBuffer.count(msgId)) {
			found += msgBuffer[msgId].falseReporters.count(reporterId);
			found += msgBuffer[msgId].trueReporters.count(reporterId);
		}
		for (auto a : splitSizes)
			if (messages[a]->count(msgId)) {
				found += (*messages[a])[msgId].falseReporters.count(reporterId);
				found += (*messages[a])[msgId].trueReporters.count(reporterId);
			}
		if (found == 1)
			rec.deletefile();
		else {
			rec.recordString("\nFAILED");
			rec.record(found);
		}
	}
	void insertMsg(int msgId) {
		if (lockedMaxId >= msgId)
			return;
		if (msgBuffer.count(msgId))
			return;
		for (int size : splitSizes)
			if (messages[size]->count(msgId))
				return;
		messageGistInDynVehMsgHistory newMsgGistObj;
		if (stageMessages)
			msgBuffer[msgId] = newMsgGistObj;
		else insertMsgObj(newMsgGistObj, msgId);
	}
	void insertEvaluation(int msgId, bool val) {
		//REDO IF USING, OUTDATED NOW
		//not used currently, can be used. currently eveluation at self is considered like any other node's report.
		if (lockedMaxId >= msgId)
			return;
		for (int size : splitSizes)
			if (messages[size]->count(msgId)) {
				messageGistInDynVehMsgHistory &msg = (*messages[size])[msgId];
				if (msg.fixed or msg.evaluated)
					return;
				splitTotals[size] -= msg.avg;
				msg.avg = val ? 1 : 0;
				splitTotals[size] += msg.avg;
				msg.evaluated = true;
				return;
			}
		messageGistInDynVehMsgHistory newMsgGistObj;
		newMsgGistObj.avg = val ? 1 : 0;
		newMsgGistObj.evaluated = true;
		insertMsgObj(newMsgGistObj, msgId);
	}
	void insertMsgObj(messageGistInDynVehMsgHistory newMsgGistObj, int msgId) {
		if (lockedMaxId >= msgId)
			return;
		recorder rec("insertMsgObjRecorder");
		rec.record(0);
		int previousSplitSize = 0;
		for (int size : splitSizes) {
			rec.recordString(std::string("\nlooping-iteration-").append(std::to_string(size)));
			auto lastElement = messages[size]->rbegin();
			if (lastElement == messages[size]->rend() or lastElement->first < msgId) {
				rec.recordString(std::string("\t-in first if-"));
				(*messages[size])[msgId] = newMsgGistObj;
				splitTotals[size] += newMsgGistObj.avg;
				rec.recordString(std::string("-end first if-"));
				if (messages[size]->size() > (size - previousSplitSize)) {
					rec.recordString(std::string("\t-in second if-"));
					newMsgGistObj = lastElement->second;
					splitTotals[size] -= newMsgGistObj.avg;
					rec.recordString(std::string("-mid second if-"));
					msgId = lastElement->first;
					messages[size]->erase(msgId);
					rec.recordString(std::string("-end second if-"));
				} else break;
			}
			previousSplitSize = size;
		}
		int found = 0;
		for (auto size : splitSizes)
			found += messages[size]->count(msgId);
		if (found == 1)
			rec.deletefile();
		else {
			rec.recordString("\nFAILED");
			rec.record(found);
		}
	}
	void ingestRSUScore(std::vector<float> vec) {
		int pendingDummyMessagesToBeInserted = vec.at(0);
		lockedMaxId = vec.at(1);
		auto bufferedMessages = getMapKeys<intSet>(msgBuffer);
		for (auto id : bufferedMessages)
			if (id <= lockedMaxId)
				msgBuffer.erase(id);
		std::map<int, messageGistInDynVehMsgHistory, std::greater<int>> newerMsgs;
		for (int i = splitSizes.size() - 1; i >= 0; --i)
			for (auto it = messages[splitSizes[i]]->begin(); ((it != messages[splitSizes[i]]->end()) and (i >= 0));
					++it) {
				if (it->first < lockedMaxId)
					break;
				else newerMsgs[it->first] = it->second;
			}
		messageGistInDynVehMsgHistory dummyMsg;
		dummyMsg.fixed = true;
		for (int j = 0; j < splitSizes.size(); ++j) {
			delete messages[splitSizes[j]];
			messages[splitSizes[j]] = new std::map<int, messageGistInDynVehMsgHistory, std::greater<int>>;
			dummyMsg.avg = vec.at(j + 2);
			int pendingDummyMessagesToBeInsertedInThisSplit = splitSizes[j] - (j ? splitSizes[j - 1] : 0);
			while (((pendingDummyMessagesToBeInserted--) > 0) and (pendingDummyMessagesToBeInsertedInThisSplit-- > 0))
				(*messages[splitSizes[j]])[pendingDummyMessagesToBeInserted] = dummyMsg;
			splitTotals[splitSizes[j]] = dummyMsg.avg
					* (splitSizes[j] - ((j ? splitSizes[j - 1] : 0) + pendingDummyMessagesToBeInsertedInThisSplit)); //short circuit above let's this work
		}
		for (auto it : newerMsgs) {
			insertMsgObj(it.second, it.first);
		}

	}
	int_2_float getSplitAvgs() {
		int_2_float splitAvgs;
		int commulative_total = 0;
		int commulative_size = 0;
		for (auto size : splitSizes) {
			if (messages[size]->size() == 0)
				break;
			commulative_total += splitTotals[size];
			commulative_size += messages[size]->size();
			splitAvgs[size] = (float) commulative_total / (float) commulative_size;
		}
		return splitAvgs;
	}
	float getMinAvg() {
		float min = 2;
		int commulative_total = 0;
		int commulative_size = 0;
		for (auto size : splitSizes) {
			if (messages[size]->size() == 0)
				break;
			commulative_total += splitTotals[size];
			commulative_size += messages[size]->size();
			float score = (float) commulative_total / (float) commulative_size;
			if (min > score)
				min = score;
		}
		return min;
	}
};
struct reportLite {
	int reporter;
	int msgId;
	reportLite(int reporter, int msgId) : reporter(reporter), msgId(msgId) {
	}
	bool operator ==(const reportLite &x) const {
		return ((reporter == x.reporter) && (msgId == x.msgId));
	}
};
struct reportValLite: public reportLite {
	bool val;
	reportValLite(int reporter, int msgId, bool val) : reportLite(reporter, msgId), val(val) {
	}
};
struct messageGistInDynVehMsgHistory2 {
	intSet trueReporters;
	intSet falseReporters;
	float avg;
	bool fixed;
	messageGistInDynVehMsgHistory2() {
		fixed = false;
		avg = -1;
	}
	void insertReport(reportValLite rep) {
		if (rep.val)
			trueReporters.insert(rep.reporter);
		else falseReporters.insert(rep.reporter);
		float t = trueReporters.size();
		float f = falseReporters.size();
		avg = t / (t + f);
	}
	void purgeBlacklisted(intSet blacklist) {
		for (auto i : blacklist) {
			trueReporters.erase(i);
			falseReporters.erase(i);
		}
		float t = trueReporters.size();
		float f = falseReporters.size();
		avg = t / (t + f);
	}
};
class split {
	int capacity;
	float total;
	std::map<int, messageGistInDynVehMsgHistory2, std::greater<int>> data;
public:
	split() {
		capacity = 0;
		total = 0;
	}
	split(int capacity) : capacity(capacity) {
		total = 0;
	}
	split(const split &s) {
		total = s.total;
		capacity = s.capacity;
		data = s.data;
	}
	void reset(int capacity) {
		this->capacity = capacity;
		total = 0;
		data.clear();
	}
	bool inline contains(int msgId) {
		return data.count(msgId);
	}
	bool inline addReport(reportValLite rep) {
		total -= data[rep.msgId].avg;
		data[rep.msgId].insertReport(rep);
		total += data[rep.msgId].avg;
	}
	void inline insert(int msgId, messageGistInDynVehMsgHistory2 msg) {
		data[msgId] = msg;
		total += msg.avg;
	}
	bool inline overflow() {
		return data.size() > capacity;
	}
	bool inline underflow() {
		return data.size() < capacity;
	}
	bool inline isEmpty() {
		return data.size() == 0;
	}
	std::pair<int, messageGistInDynVehMsgHistory2> inline popBack() {
		auto last = data.rbegin();
		total -= last->second.avg;
		std::pair<int, messageGistInDynVehMsgHistory2> retval(last->first, last->second);
		data.erase(retval.first);
		return retval;
	}
	std::pair<int, messageGistInDynVehMsgHistory2> inline popFront() {
		auto first = data.begin();
		total -= first->second.avg;
		std::pair<int, messageGistInDynVehMsgHistory2> retval(first->first, first->second);
		data.erase(retval.first);
		return retval;
	}
	std::pair<int, messageGistInDynVehMsgHistory2> inline popMsg(int msgId) {
		total -= data[msgId].avg;
		std::pair<int, messageGistInDynVehMsgHistory2> retval(msgId, data[msgId]);
		data.erase(msgId);
		return retval;
	}
	bool inline isFixed(int mid) {
		return data[mid].fixed;
	}
	float inline getTotal() {
		return total;
	}
	int inline getSize() {
		return data.size();
	}
	int inline getMsgReportersSize(int msgId) {
		return data[msgId].falseReporters.size() + data[msgId].trueReporters.size();
	}
	float inline getAvg() {
		return (float) total / (float) data.size();
	}
	void inline purgeBlacklisted(intSet blacklist) {
		for (auto &i : data)
			i.second.purgeBlacklisted(blacklist);
	}
	/*void purgeLessThan(int min) {
	 std::vector<reportValLite> datanew;
	 for (auto it : data)
	 if (it.msgId > min)
	 datanew.insert(datanew.end(), it);
	 else --total;
	 data = datanew;
	 }*/
};
struct vehMsgHistoryDynamic2 {
	bool stageMessages;
	int stagingThreshhold;
	split msgBuffer;
	int messagesRecv;
	int reportsRecv;
	int logLimit;
	int splitFactor;
	int splitLevel;
	int lockedMaxId; //if the rsu has sent evaluations of messages upto this message Id then we ignore messages and reports on messages less then it.
	std::tr1::unordered_map<int, split> splits; //ordered map
	int_2_float splitTotals;
	std::vector<int> splitSizes;
	std::map<int, bool> myReports;
	vehMsgHistoryDynamic2(int start, int factor, int level, bool staging = false, int stageingTh = 5) { //for f=5,l=4 : sS=<{5,-1},{25,-1},{125,-1},{625,-1}> , lL=625.
		splitFactor = factor;
		splitLevel = level;
		messagesRecv = 0;
		reportsRecv = 0;
		lockedMaxId = -1;
		splitSizes = calculatePowersAscending<std::vector<int>>(start, factor, level);
		int previousSize = 0;
		for (int size : splitSizes) {
			splits[size] = split(size - previousSize);
			logLimit = size;
			previousSize = size;
		}
		msgBuffer = split(5);
	}
	void inline insertMyReport(int reporterId, int msgId, bool val) {
		myReports[msgId] = val;
		reportValLite rep(reporterId, msgId, val);
		insertReport(reporterId, msgId, val);
	}
	void inline insertReport(int reporterId, int msgId, bool val) {
		reportValLite rep(reporterId, msgId, val);
		insertReport(rep);
	}
	void inline insertReport(reportValLite rep) {
		bool foundFlag = false;
		if (stageMessages && (msgBuffer.contains(rep.msgId))) {
			msgBuffer.addReport(rep);
			if (msgBuffer.getMsgReportersSize(rep.msgId) >= stagingThreshhold) {
				auto msg = msgBuffer.popMsg(rep.msgId);
				insertMsg(msg);
			}
			foundFlag = true;
		}
		for (auto size : splitSizes) {
			if (splits[size].contains(rep.msgId)) {
				if (!splits[size].isFixed(rep.msgId))
					splits[size].addReport(rep);
				foundFlag = true;
			}
		}
		if (!foundFlag) {
			std::pair<int, messageGistInDynVehMsgHistory2> newMsg;
			newMsg.first = rep.msgId;
			newMsg.second.insertReport(rep);
			if (stageMessages)
				msgBuffer.insert(newMsg.first, newMsg.second);
			else insertMsg(newMsg);
		}
	}
	void inline insertMsg(std::pair<int, messageGistInDynVehMsgHistory2> newMsg) {
		for (auto size : splitSizes) {
			splits[size].insert(newMsg.first, newMsg.second);
			if (splits[size].overflow())
				newMsg = splits[size].popBack();
			else break;
		}
	}
	void inline ingestRSUScore(std::vector<float> vec, intSet blacklist) {
		int pendingDummyMessagesToBeInserted = vec.at(0);
		if (vec.at(1) < lockedMaxId)
			lockedMaxId = vec.at(1);
		else return;
		std::tr1::unordered_map<int, split> oldSplits;
		int previousSize = 0;
		for (auto size : splitSizes) {
			oldSplits[size] = splits[size];
			splits[size].reset(size - previousSize);
			previousSize = size;
		}
		messageGistInDynVehMsgHistory2 fixedMessage;
		fixedMessage.fixed = true;
		for (int j = 0; j < splitSizes.size(); ++j) {
			float scoreToBeEmulated = vec.at(j + 2);
			if (scoreToBeEmulated == -1)
				break;
			if (j) // score for this split -> score for this split-previous split; // in rsu app the splits score overlap here we overlap them while outputtin the result;
				scoreToBeEmulated = (((float) splitFactor * scoreToBeEmulated) - vec.at(j + 1))
						/ (float) (splitFactor - 1);
			fixedMessage.avg = scoreToBeEmulated;
			int messagesToBeEmulated = splitSizes[j] - j ? splitSizes[j - 1] : 0;
			int messagesAdded = 0;
			while (messagesAdded++ < messagesToBeEmulated)
				splits[splitSizes[j]].insert(lockedMaxId - messagesAdded + 1, fixedMessage);
		}
		for (auto size : splitSizes) {
			while (!oldSplits[size].isEmpty()) {
				std::pair<int, messageGistInDynVehMsgHistory2> msg = oldSplits[size].popBack();
				if (msg.first > lockedMaxId) {
					msg.second.purgeBlacklisted(blacklist);
					insertMsg(msg);
				} else {
					myReports.erase(msg.first);
				}
			}
		}
		if (stageMessages)
			msgBuffer.purgeBlacklisted(blacklist);
	}
	int_2_float getSplitAvgs() {
		int_2_float splitAvgs;
		if (splits[splitSizes[0]].getSize()) { //FIXME if (!splits[splitSizes[0]].underflow())
			float commulative_total = 0;
			float commulative_size = 0;
			for (auto size : splitSizes) {
				commulative_total += splits[size].getTotal();
				commulative_size += splits[size].getSize();
				if (!commulative_size)
					break;
				splitAvgs[size] = commulative_total / commulative_size;
			}
		}
		return splitAvgs;
	}
	float getMinAvg() {
		if (splits[splitSizes[0]].getSize() == 0)
			return 2;
		float min = 2;
		float commulative_total = 0;
		float commulative_size = 0;
		for (auto size : splitSizes) {
			if (splits[size].getSize() == 0)
				break;
			commulative_total += splits[size].getTotal();
			commulative_size += splits[size].getSize();
			float score = commulative_total / commulative_size;
			if (min > score)
				min = score;
		}
		return min;
	}
};

class reportLiteHasher {
public:
	std::size_t operator()(const reportLite &x) const {
		return std::hash<int>()(x.reporter) ^ std::hash<int>()(~x.msgId);
	}
};
class splitLite {
	int capacity;
	int total;
	std::vector<reportValLite> data;
public:
	splitLite() {
		capacity = 0;
		total = 0;
	}
	splitLite(int capacity) : capacity(capacity) {
		total = 0;
	}
	splitLite(const splitLite &s) {
		total = s.total;
		capacity = s.capacity;
		data = s.data;
	}
	void reset(int capacity) {
		this->capacity = capacity;
		total = 0;
		data.clear();
	}
	void inline insert(reportValLite rep) {
		data.insert(data.begin(), rep);
		if (rep.val)
			++total;
	}
	bool inline overflow() {
		return data.size() > capacity;
	}
	bool inline underflow() {
		return data.size() < capacity;
	}
	bool inline isEmpty() {
		return data.size() == 0;
	}
	reportValLite inline popBack() {
		auto last = data.rbegin();
		if (last->val)
			--total;
		reportValLite rep = *last;
		data.pop_back();
		return rep;
	}
	reportValLite inline popFront() {
		auto first = data.begin();
		if (first->val)
			--total;
		reportValLite rep = *first;
		data.erase(first);
		return rep;
	}
	int getTotal() {
		return total;
	}
	int getSize() {
		return data.size();
	}
	float getAvg() {
		return (float) total / (float) data.size();
	}
	void purgeLessThan(int min) {
		std::vector<reportValLite> datanew;
		for (auto it : data)
			if (it.msgId > min)
				datanew.insert(datanew.end(), it);
			else --total;
		data = datanew;
	}
};
struct vehMsgHistoryDynamic_Lite {
	int lockedMaxId;
	int messagesRecv;
	int reportsRecv;
	int logLimit;
	int splitFactor;
	int splitLevel;
	int_2_float splitTotals;
	std::vector<int> splitSizes;
	std::map<int, bool> myReports;
	std::tr1::unordered_set<reportLite, reportLiteHasher> reportsSet;
	std::tr1::unordered_map<int, splitLite> splits;
	vehMsgHistoryDynamic_Lite(int start, int factor, int level) { //for f=5,l=4 : sS=<{5,-1},{25,-1},{125,-1},{625,-1}> , lL=625.
		splitFactor = factor;
		splitLevel = level;
		messagesRecv = 0;
		reportsRecv = 0;
		lockedMaxId = -1;
		splitSizes = calculatePowersAscending<std::vector<int>>(start, factor, level);
		int previousSize = 0;
		for (int size : splitSizes) {
			splits[size] = splitLite(size - previousSize);
			logLimit = size;
			previousSize = size;
		}
	}
	void insertMyReport(int reporterId, int msgId, bool val) {
		myReports[msgId] = val;
		insertReport(reporterId, msgId, val);
	}
	void insertReport(int reporterId, int msgId, bool val) {
		reportLite rep(reporterId, msgId);
		if (reportsSet.count(rep))
			return;
		reportValLite repval(reporterId, msgId, val);
		insertReport(repval);
		reportsSet.insert(rep);
	}
	void insertReport(reportValLite rep) {
		for (auto size : splitSizes) {
			splits[size].insert(rep);
			if (splits[size].overflow())
				rep = splits[size].popBack();
			else break;
		}
	}
	void ingestRSUScore(std::vector<float> vec, intSet blacklist) {
		int pendingDummyMessagesToBeInserted = vec.at(0);
		if (vec.at(1) < lockedMaxId)
			lockedMaxId = vec.at(1);
		else return;
		float reportDensity = vec.at(2);
		std::tr1::unordered_map<int, splitLite> oldSplits;
		int previousSize = 0;
		for (auto size : splitSizes) {
			oldSplits[size] = splits[size];
			splits[size] = splitLite((size - previousSize) * reportDensity);
			previousSize = size;
		}
		reportValLite trueRep(-1, -1, true);
		reportValLite falseRep(-1, -1, false);
		for (int j = 0; j < splitSizes.size(); ++j) {
			float scoreToBeEmulated = vec.at(j + 3);
			if (scoreToBeEmulated == -1)
				break;
			if (j) // score for this split -> score for this split-previous split; // in rsu app the splits score overlap here we overlap them while outputtin the result;
				scoreToBeEmulated = (((float) splitFactor * scoreToBeEmulated) - vec.at(j + 2))
						/ (float) (splitFactor - 1);
			int messagesToBeEmulated = splitSizes[j] - j ? splitSizes[j - 1] : 0;
			int reportsToBeAdded = messagesToBeEmulated * reportDensity;
			float emulatedScore = 0.5;
			int trueReportsAdded = 0;
			int reportsAdded = 0;
			while (reportsAdded++ < reportsToBeAdded) {
				if (emulatedScore < scoreToBeEmulated) {
					splits[splitSizes[j]].insert(trueRep);
					++trueReportsAdded;
				} else splits[splitSizes[j]].insert(falseRep);
				emulatedScore = (float) trueReportsAdded / (float) reportsAdded;
			}
		}
		for (auto size : splitSizes) {
			while (!oldSplits[size].isEmpty()) {
				reportValLite rep = oldSplits[size].popBack();
				if ((rep.msgId > lockedMaxId) && (blacklist.count(rep.reporter) == 0))
					insertReport(rep);
				else {
					reportsSet.erase(reportLite(rep.reporter, rep.msgId));
					myReports.erase(rep.msgId);
				}
			}
		}
	}
	int_2_float getSplitAvgs() {
		int_2_float splitAvgs;
		if (splits[splitSizes[0]].getSize()) {
			int commulative_total = 0;
			int commulative_size = 0;
			for (auto size : splitSizes) {
				commulative_total += splits[size].getTotal();
				commulative_size += splits[size].getSize();
				if (!commulative_size)
					break;
				splitAvgs[size] = (float) commulative_total / (float) commulative_size;
			}
		}
		return splitAvgs;
	}
	float getMinAvg() {
		if (splits[splitSizes[0]].getSize() == 0)
			return 0;
		float min = 2;
		int commulative_total = 0;
		int commulative_size = 0;
		for (auto size : splitSizes) {
			if (splits[size].getSize() == 0)
				break;
			commulative_total += splits[size].getTotal();
			commulative_size += splits[size].getSize();
			float score = (float) commulative_total / (float) commulative_size;
			if (min > score)
				min = score;
		}
		return min;
	}
};

typedef std::tr1::unordered_map<int, vehMsgHistory*> int2VehMsgHistory;
typedef std::tr1::unordered_map<int, vehMsgHistoryDynamic*> int2vehMsgHistoryDynamic;
typedef std::tr1::unordered_map<int, vehMsgHistoryDynamic_Lite*> int2vehMsgHistoryDynamic_Lite;
typedef std::tr1::unordered_map<int, vehMsgHistoryDynamic2*> int2vehMsgHistoryDynamic2;
#endif
