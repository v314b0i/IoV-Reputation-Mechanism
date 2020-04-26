#ifndef MY_AUX_HEADER_FILE
#define MY_AUX_HEADER_FILE

#include <tr1/unordered_map>
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
typedef std::tr1::unordered_map<int, bool> int_2_bool;
typedef std::tr1::unordered_map<int, float> int_2_float;
typedef std::tr1::unordered_map<int, int> int_2_int;
typedef std::tr1::unordered_set<int> intSet;
typedef std::tr1::unordered_map<int, int_2_float> int_2_int2float;
typedef std::tr1::unordered_map<int, int_2_bool*> int_2_int2bool;
typedef std::tr1::unordered_map<int, intSet*> int_2_intSet;
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
template<typename type>inline float vectorMean(std::vector<type> vec) {
	if (vec.size() == 0)
		return -1;
	float acc = 0;
	for (auto i : vec)
		acc += i;
	return acc / (float) vec.size();
}
template<typename type>inline float vectorMode(std::vector<type> vec) {
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
template<typename type>inline float standardDeviation(std::vector<type> v, float centre) {
	float acc = 0;
	float diff;
	for (auto it : v) {
		diff = it - centre;
		acc += diff * diff;
	}
	return sqrt(acc / (float) v.size());
}
template<typename type>inline float medianAbsoluteDeviation(std::vector<type> v, float centre) {
	std::vector<float> vec;
	for (auto it : v)
		vec.insert(vec.end(), it > centre ? it - centre : centre - it);
	return vectorMedian(vec);
}

template<typename type>inline bool ismultimodal(std::vector<type> vec, float median) {
	//FIXME currently is an approximate testing function written by me. Should instead implement a proper test such as Hartigan's dip test
	//https://en.wikipedia.org/wiki/Multimodal_distribution#General_tests
	float mode = vectorMode(vec);
	float diffBetMedianAndMode = median > mode ? median - mode : mode - median;
	return diffBetMedianAndMode >= 0.20; // arbitrary cut-off. Use hartigans'd dip test and set cut off=0.1 on the p value.
	//should not consider the scores from nodes that had few total reports.
}
template<typename type>inline bool ismultimodal(std::vector<type> vec) {
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
struct vehMsgHistory {
	//can use ordered int->float map for messages instead of vector<{int,float}> but insertion and deletion complexity would be log(n)
	//would have to use it if we allow for older messges to be added that were skipped earlier (highly exceptional cases) but that adds
	//more complexity to the functions, not required for simulation.
	int logLimit;
	int splitFactor;
	int splitLevel;
	int totalMessages;
	std::vector<msgVal> messages;
	int_2_float splitTotals;
	intSet splitSizes;
	vehMsgHistory(int factor, int level) { //for f=5,l=4 : sS=<{5,-1},{25,-1},{125,-1},{625,-1}> , lL=625.
		splitFactor = factor;
		splitLevel = level;
		int size = 1;
		for (int i = 1; i <= level; i++) {
			size *= splitFactor;
			splitSizes.insert(size);
			splitTotals[size] = 0;
		}
		logLimit = size;
	}

	void insert(int_2_float messageValsMap) {
		//FIXME std::vector<int> keys=getMapKeys<std::vector<int>>(messageValsMap);
		//
		std::vector<int> keys;
		for (auto i : messageValsMap)
			keys.insert(keys.end(), i.first);
		//
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
};
typedef std::tr1::unordered_map<int, vehMsgHistory*> int2VehMsgHistory;
#endif
