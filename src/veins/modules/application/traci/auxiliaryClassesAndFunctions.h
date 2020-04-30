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

//TODO create separate structs for stats collection counters, keep app functionality and ststs collection differentiated.
enum MyApplMessageKinds {
	SEND_INFOMSG_EVT, STAGE_SHIFT_EVT, INFO_MSG, REPORT_MSG
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
	//can use ordered int->float map for messages instead of vector<{int,float}> but insertion and deletion complexity would be log(n)
	//would have to use it if we allow for older messges to be added that were skipped earlier (highly exceptional cases) but that adds
	//more complexity to the functions, not required for simulation.
	int messagesRecv;
	int reportsRecv;
	int logLimit;
	int splitFactor;
	int splitLevel;
	int lockedMaxId; //if the rsu has sent evaluations of messages upto this message Id then we ignore messages and reports on messages less then it.
	std::tr1::unordered_map<int, std::map<int, messageGistInDynVehMsgHistory, std::greater<int>>*> messages; //ordered map
	int_2_float splitTotals;
	std::vector<int> splitSizes;
	vehMsgHistoryDynamic(int start, int factor, int level) { //for f=5,l=4 : sS=<{5,-1},{25,-1},{125,-1},{625,-1}> , lL=625.
		splitFactor = factor;
		splitLevel = level;
		messagesRecv = 0;
		reportsRecv = 0;
		lockedMaxId = 0;
		splitSizes = calculatePowersAscending<std::vector<int>>(start, factor, level);
		for (int size : splitSizes) {
			messages[size] = new std::map<int, messageGistInDynVehMsgHistory, std::greater<int>>;
			splitTotals[size] = 0;
			logLimit = size;
		}
	}
	void insertReport(int reporterId, int msgId, bool val) {
		if (lockedMaxId >= msgId)
			return;
		for (int size : splitSizes)
			if (messages[size]->count(msgId)) {
				messageGistInDynVehMsgHistory &msg = (*messages[size])[msgId];
				if (msg.fixed or msg.evaluated)
					return;
				if (val)
					msg.trueReporters.insert(reporterId);
				else msg.falseReporters.insert(reporterId);
				float avg = (float) msg.trueReporters.size();
				avg /= avg + (float) msg.falseReporters.size();
				splitTotals[size] -= msg.avg;
				splitTotals[size] += avg;
				msg.avg = avg;
				return;
			}
		messageGistInDynVehMsgHistory newMsgGistObj;
		newMsgGistObj.avg = val ? 1 : 0;
		if (val)
			newMsgGistObj.trueReporters.insert(reporterId);
		else newMsgGistObj.falseReporters.insert(reporterId);
		insertMsgObj(newMsgGistObj, msgId);
	}
	void insertMsg(int msgId) {
		if (lockedMaxId >= msgId)
			return;
		for (int size : splitSizes)
			if (messages[size]->count(msgId))
				return;
		messageGistInDynVehMsgHistory newMsgGistObj;
		insertMsgObj(newMsgGistObj, msgId);
	}
	void insertEvaluation(int msgId, bool val) {
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
		for (int size : splitSizes) {
			if ((--(*messages[size]).end())->first < msgId) {
				(*messages[size])[msgId] = newMsgGistObj;
				splitTotals[size] += newMsgGistObj.avg;
				if ((*messages[size]).size() > size) {
					newMsgGistObj = (--(*messages[size]).end())->second;
					splitTotals[size] -= newMsgGistObj.avg;
					msgId = (--(*messages[size]).end())->first;
					(*messages[size]).erase(msgId);
				} else break;
			}
		}
	}
	void ingestRSUScore(std::vector<float> vec) {
		int pendingDummyMessagesToBeInserted = vec.at(0);
		lockedMaxId = vec.at(1);
		std::map<int, messageGistInDynVehMsgHistory, std::greater<int>> newerMsgs;
		for (int i = splitSizes.size() - 1; i >= 0; --i)
			for (auto it = messages[splitSizes[i]]->begin(); ((it != messages[splitSizes[i]]->end()) and (i>=0)); ++it) {
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
			int size = splitSizes[j];
			while (((pendingDummyMessagesToBeInserted--) > 0) and (size-- > 0))
				(*messages[splitSizes[j]])[pendingDummyMessagesToBeInserted] = dummyMsg;
			splitTotals[splitSizes[j]] = dummyMsg.avg * (splitSizes[j] - size); //short circuit above let's this work
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
			commulative_total += splitTotals[size];
			commulative_size += messages[size]->size();
			splitAvgs[size] = (float) commulative_total / (float) commulative_size;
		}
		return splitAvgs;
	}
};
typedef std::tr1::unordered_map<int, vehMsgHistory*> int2VehMsgHistory;
typedef std::tr1::unordered_map<int, vehMsgHistoryDynamic*> int2vehMsgHistoryDynamic;
#endif
