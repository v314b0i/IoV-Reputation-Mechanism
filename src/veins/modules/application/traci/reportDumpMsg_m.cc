//
// Generated file, do not edit! Created by nedtool 5.6 from veins/modules/application/traci/reportDumpMsg.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Wshadow"
#  pragma clang diagnostic ignored "-Wconversion"
#  pragma clang diagnostic ignored "-Wunused-parameter"
#  pragma clang diagnostic ignored "-Wc++98-compat"
#  pragma clang diagnostic ignored "-Wunreachable-code-break"
#  pragma clang diagnostic ignored "-Wold-style-cast"
#elif defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wshadow"
#  pragma GCC diagnostic ignored "-Wconversion"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#  pragma GCC diagnostic ignored "-Wold-style-cast"
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#  pragma GCC diagnostic ignored "-Wfloat-conversion"
#endif

#include <iostream>
#include <sstream>
#include "reportDumpMsg_m.h"

namespace omnetpp {

// Template pack/unpack rules. They are declared *after* a1l type-specific pack functions for multiple reasons.
// They are in the omnetpp namespace, to allow them to be found by argument-dependent lookup via the cCommBuffer argument

// Packing/unpacking an std::vector
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::vector<T,A>& v)
{
    int n = v.size();
    doParsimPacking(buffer, n);
    for (int i = 0; i < n; i++)
        doParsimPacking(buffer, v[i]);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::vector<T,A>& v)
{
    int n;
    doParsimUnpacking(buffer, n);
    v.resize(n);
    for (int i = 0; i < n; i++)
        doParsimUnpacking(buffer, v[i]);
}

// Packing/unpacking an std::list
template<typename T, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::list<T,A>& l)
{
    doParsimPacking(buffer, (int)l.size());
    for (typename std::list<T,A>::const_iterator it = l.begin(); it != l.end(); ++it)
        doParsimPacking(buffer, (T&)*it);
}

template<typename T, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::list<T,A>& l)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        l.push_back(T());
        doParsimUnpacking(buffer, l.back());
    }
}

// Packing/unpacking an std::set
template<typename T, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::set<T,Tr,A>& s)
{
    doParsimPacking(buffer, (int)s.size());
    for (typename std::set<T,Tr,A>::const_iterator it = s.begin(); it != s.end(); ++it)
        doParsimPacking(buffer, *it);
}

template<typename T, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::set<T,Tr,A>& s)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        T x;
        doParsimUnpacking(buffer, x);
        s.insert(x);
    }
}

// Packing/unpacking an std::map
template<typename K, typename V, typename Tr, typename A>
void doParsimPacking(omnetpp::cCommBuffer *buffer, const std::map<K,V,Tr,A>& m)
{
    doParsimPacking(buffer, (int)m.size());
    for (typename std::map<K,V,Tr,A>::const_iterator it = m.begin(); it != m.end(); ++it) {
        doParsimPacking(buffer, it->first);
        doParsimPacking(buffer, it->second);
    }
}

template<typename K, typename V, typename Tr, typename A>
void doParsimUnpacking(omnetpp::cCommBuffer *buffer, std::map<K,V,Tr,A>& m)
{
    int n;
    doParsimUnpacking(buffer, n);
    for (int i=0; i<n; i++) {
        K k; V v;
        doParsimUnpacking(buffer, k);
        doParsimUnpacking(buffer, v);
        m[k] = v;
    }
}

// Default pack/unpack function for arrays
template<typename T>
void doParsimArrayPacking(omnetpp::cCommBuffer *b, const T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimPacking(b, t[i]);
}

template<typename T>
void doParsimArrayUnpacking(omnetpp::cCommBuffer *b, T *t, int n)
{
    for (int i = 0; i < n; i++)
        doParsimUnpacking(b, t[i]);
}

// Default rule to prevent compiler from choosing base class' doParsimPacking() function
template<typename T>
void doParsimPacking(omnetpp::cCommBuffer *, const T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimPacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

template<typename T>
void doParsimUnpacking(omnetpp::cCommBuffer *, T& t)
{
    throw omnetpp::cRuntimeError("Parsim error: No doParsimUnpacking() function for type %s", omnetpp::opp_typename(typeid(t)));
}

}  // namespace omnetpp

namespace veins {

// forward
template<typename T, typename A>
std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec);

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

// operator<< for std::vector<T>
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

Register_Class(reportDumpMsg)

reportDumpMsg::reportDumpMsg(const char *name, short kind) : ::veins::BaseFrame1609_4(name,kind)
{
    this->senderAddress = -1;
    this->reporteeAddress = -1;
    this->primaryRecipientAddress = -1;
}

reportDumpMsg::reportDumpMsg(const reportDumpMsg& other) : ::veins::BaseFrame1609_4(other)
{
    copy(other);
}

reportDumpMsg::~reportDumpMsg()
{
}

reportDumpMsg& reportDumpMsg::operator=(const reportDumpMsg& other)
{
    if (this==&other) return *this;
    ::veins::BaseFrame1609_4::operator=(other);
    copy(other);
    return *this;
}

void reportDumpMsg::copy(const reportDumpMsg& other)
{
    this->senderAddress = other.senderAddress;
    this->reporteeAddress = other.reporteeAddress;
    this->primaryRecipientAddress = other.primaryRecipientAddress;
    this->trueMsgs = other.trueMsgs;
    this->falseMsgs = other.falseMsgs;
}

void reportDumpMsg::parsimPack(omnetpp::cCommBuffer *b) const
{
    ::veins::BaseFrame1609_4::parsimPack(b);
    doParsimPacking(b,this->senderAddress);
    doParsimPacking(b,this->reporteeAddress);
    doParsimPacking(b,this->primaryRecipientAddress);
    doParsimPacking(b,this->trueMsgs);
    doParsimPacking(b,this->falseMsgs);
}

void reportDumpMsg::parsimUnpack(omnetpp::cCommBuffer *b)
{
    ::veins::BaseFrame1609_4::parsimUnpack(b);
    doParsimUnpacking(b,this->senderAddress);
    doParsimUnpacking(b,this->reporteeAddress);
    doParsimUnpacking(b,this->primaryRecipientAddress);
    doParsimUnpacking(b,this->trueMsgs);
    doParsimUnpacking(b,this->falseMsgs);
}

LAddress::L2Type& reportDumpMsg::getSenderAddress()
{
    return this->senderAddress;
}

void reportDumpMsg::setSenderAddress(const LAddress::L2Type& senderAddress)
{
    this->senderAddress = senderAddress;
}

LAddress::L2Type& reportDumpMsg::getReporteeAddress()
{
    return this->reporteeAddress;
}

void reportDumpMsg::setReporteeAddress(const LAddress::L2Type& reporteeAddress)
{
    this->reporteeAddress = reporteeAddress;
}

LAddress::L2Type& reportDumpMsg::getPrimaryRecipientAddress()
{
    return this->primaryRecipientAddress;
}

void reportDumpMsg::setPrimaryRecipientAddress(const LAddress::L2Type& primaryRecipientAddress)
{
    this->primaryRecipientAddress = primaryRecipientAddress;
}

const char * reportDumpMsg::getTrueMsgs() const
{
    return this->trueMsgs.c_str();
}

void reportDumpMsg::setTrueMsgs(const char * trueMsgs)
{
    this->trueMsgs = trueMsgs;
}

const char * reportDumpMsg::getFalseMsgs() const
{
    return this->falseMsgs.c_str();
}

void reportDumpMsg::setFalseMsgs(const char * falseMsgs)
{
    this->falseMsgs = falseMsgs;
}

class reportDumpMsgDescriptor : public omnetpp::cClassDescriptor
{
  private:
    mutable const char **propertynames;
  public:
    reportDumpMsgDescriptor();
    virtual ~reportDumpMsgDescriptor();

    virtual bool doesSupport(omnetpp::cObject *obj) const override;
    virtual const char **getPropertyNames() const override;
    virtual const char *getProperty(const char *propertyname) const override;
    virtual int getFieldCount() const override;
    virtual const char *getFieldName(int field) const override;
    virtual int findField(const char *fieldName) const override;
    virtual unsigned int getFieldTypeFlags(int field) const override;
    virtual const char *getFieldTypeString(int field) const override;
    virtual const char **getFieldPropertyNames(int field) const override;
    virtual const char *getFieldProperty(int field, const char *propertyname) const override;
    virtual int getFieldArraySize(void *object, int field) const override;

    virtual const char *getFieldDynamicTypeString(void *object, int field, int i) const override;
    virtual std::string getFieldValueAsString(void *object, int field, int i) const override;
    virtual bool setFieldValueAsString(void *object, int field, int i, const char *value) const override;

    virtual const char *getFieldStructName(int field) const override;
    virtual void *getFieldStructValuePointer(void *object, int field, int i) const override;
};

Register_ClassDescriptor(reportDumpMsgDescriptor)

reportDumpMsgDescriptor::reportDumpMsgDescriptor() : omnetpp::cClassDescriptor("veins::reportDumpMsg", "veins::BaseFrame1609_4")
{
    propertynames = nullptr;
}

reportDumpMsgDescriptor::~reportDumpMsgDescriptor()
{
    delete[] propertynames;
}

bool reportDumpMsgDescriptor::doesSupport(omnetpp::cObject *obj) const
{
    return dynamic_cast<reportDumpMsg *>(obj)!=nullptr;
}

const char **reportDumpMsgDescriptor::getPropertyNames() const
{
    if (!propertynames) {
        static const char *names[] = {  nullptr };
        omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
        const char **basenames = basedesc ? basedesc->getPropertyNames() : nullptr;
        propertynames = mergeLists(basenames, names);
    }
    return propertynames;
}

const char *reportDumpMsgDescriptor::getProperty(const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : nullptr;
}

int reportDumpMsgDescriptor::getFieldCount() const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount() : 5;
}

unsigned int reportDumpMsgDescriptor::getFieldTypeFlags(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeFlags(field);
        field -= basedesc->getFieldCount();
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<5) ? fieldTypeFlags[field] : 0;
}

const char *reportDumpMsgDescriptor::getFieldName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldName(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldNames[] = {
        "senderAddress",
        "reporteeAddress",
        "primaryRecipientAddress",
        "trueMsgs",
        "falseMsgs",
    };
    return (field>=0 && field<5) ? fieldNames[field] : nullptr;
}

int reportDumpMsgDescriptor::findField(const char *fieldName) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount() : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "senderAddress")==0) return base+0;
    if (fieldName[0]=='r' && strcmp(fieldName, "reporteeAddress")==0) return base+1;
    if (fieldName[0]=='p' && strcmp(fieldName, "primaryRecipientAddress")==0) return base+2;
    if (fieldName[0]=='t' && strcmp(fieldName, "trueMsgs")==0) return base+3;
    if (fieldName[0]=='f' && strcmp(fieldName, "falseMsgs")==0) return base+4;
    return basedesc ? basedesc->findField(fieldName) : -1;
}

const char *reportDumpMsgDescriptor::getFieldTypeString(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldTypeString(field);
        field -= basedesc->getFieldCount();
    }
    static const char *fieldTypeStrings[] = {
        "LAddress::L2Type",
        "LAddress::L2Type",
        "LAddress::L2Type",
        "string",
        "string",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : nullptr;
}

const char **reportDumpMsgDescriptor::getFieldPropertyNames(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldPropertyNames(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

const char *reportDumpMsgDescriptor::getFieldProperty(int field, const char *propertyname) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldProperty(field, propertyname);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        default: return nullptr;
    }
}

int reportDumpMsgDescriptor::getFieldArraySize(void *object, int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldArraySize(object, field);
        field -= basedesc->getFieldCount();
    }
    reportDumpMsg *pp = (reportDumpMsg *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

const char *reportDumpMsgDescriptor::getFieldDynamicTypeString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldDynamicTypeString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    reportDumpMsg *pp = (reportDumpMsg *)object; (void)pp;
    switch (field) {
        default: return nullptr;
    }
}

std::string reportDumpMsgDescriptor::getFieldValueAsString(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldValueAsString(object,field,i);
        field -= basedesc->getFieldCount();
    }
    reportDumpMsg *pp = (reportDumpMsg *)object; (void)pp;
    switch (field) {
        case 0: {std::stringstream out; out << pp->getSenderAddress(); return out.str();}
        case 1: {std::stringstream out; out << pp->getReporteeAddress(); return out.str();}
        case 2: {std::stringstream out; out << pp->getPrimaryRecipientAddress(); return out.str();}
        case 3: return oppstring2string(pp->getTrueMsgs());
        case 4: return oppstring2string(pp->getFalseMsgs());
        default: return "";
    }
}

bool reportDumpMsgDescriptor::setFieldValueAsString(void *object, int field, int i, const char *value) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->setFieldValueAsString(object,field,i,value);
        field -= basedesc->getFieldCount();
    }
    reportDumpMsg *pp = (reportDumpMsg *)object; (void)pp;
    switch (field) {
        case 3: pp->setTrueMsgs((value)); return true;
        case 4: pp->setFalseMsgs((value)); return true;
        default: return false;
    }
}

const char *reportDumpMsgDescriptor::getFieldStructName(int field) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructName(field);
        field -= basedesc->getFieldCount();
    }
    switch (field) {
        case 0: return omnetpp::opp_typename(typeid(LAddress::L2Type));
        case 1: return omnetpp::opp_typename(typeid(LAddress::L2Type));
        case 2: return omnetpp::opp_typename(typeid(LAddress::L2Type));
        default: return nullptr;
    };
}

void *reportDumpMsgDescriptor::getFieldStructValuePointer(void *object, int field, int i) const
{
    omnetpp::cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount())
            return basedesc->getFieldStructValuePointer(object, field, i);
        field -= basedesc->getFieldCount();
    }
    reportDumpMsg *pp = (reportDumpMsg *)object; (void)pp;
    switch (field) {
        case 0: return (void *)(&pp->getSenderAddress()); break;
        case 1: return (void *)(&pp->getReporteeAddress()); break;
        case 2: return (void *)(&pp->getPrimaryRecipientAddress()); break;
        default: return nullptr;
    }
}

} // namespace veins

