/*
 *  Copyright (C) 2017
 *  Andreas Walz [andreas.walz@hs-offenburg.de]
 *  Offenburg University of Applied Sciences
 *  Institute of Reliable Embedded Systems and Communications Electronics (ivESK)
 *  [https://ivesk.hs-offenburg.de/]
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <iostream>
#include "DataUnit.h"
#include "VectorBuffer.h"
#include "String_.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;


/*
 * ___________________________________________________________________________
 */
TypeRegistry& TypeRegistry::getInstance() {

    static TypeRegistry* singleton = (TypeRegistry*)0;
    if (singleton == 0) {
        singleton = new TypeRegistry();
    }
    return *singleton;
}


/*
 * ___________________________________________________________________________
 */
TypeRegistry::TypeRegistry() {
}


/*
 * ___________________________________________________________________________
 */
bool TypeRegistry::add(const TypeDescriptor& typeDescriptor) {

    bool success = false;

    /* prevent duplicates in registry */
    if (this->findById(typeDescriptor.id()) == 0 &&
            this->findByName(typeDescriptor.name()) == 0) {

        /* add to registry */
        registry_.push_back(&typeDescriptor);

        success = true;
    }

    return success;
}


/*
 * ___________________________________________________________________________
 */
const TypeDescriptor* TypeRegistry::findByName(
        const string& typeName) {

    const TypeDescriptor* desc = (TypeDescriptor*)0;
    size_t n = registry_.size();
    size_t i = 0;

    while (i < n && desc == 0) {
        if (registry_[i]->name() == typeName) {
            desc = registry_[i];
        }
        ++i;
    }

    return desc;
}


/*
 * ___________________________________________________________________________
 */
const TypeDescriptor* TypeRegistry::findById(size_t typeId) {

    const TypeDescriptor* desc = (TypeDescriptor*)0;
    size_t n = registry_.size();
    size_t i = 0;

    while (i < n && desc == 0) {
        if (registry_[i]->id() == typeId) {
            desc = registry_[i];
        }
        ++i;
    }

    return desc;
}


/*
 * ___________________________________________________________________________
 */
size_t TypeRegistry::print() {

    /* iterate over entries in registry */
    for (registry_t::iterator it = registry_.begin();
            it != registry_.end(); ++it) {

        /* print typeId and type name */
        cout << String::format("[%5d] %s", (*it)->id(),
                String::makeBoldBlue((*it)->name()).c_str());

        /* point to base type (where applicable) */
        const TypeDescriptor* base = (*it)->getBaseTypeDescriptor();
        if (base != 0) {
            cout << String::format(" ==> %s", base->name().c_str());
        }

        cout << endl;
    }

    return registry_.size();
}


/*
 * ___________________________________________________________________________
 */
TypeRegistry::~TypeRegistry() {
}


/*
 * ===========================================================================
 */


/*
 * ___________________________________________________________________________
 */
TypeDescriptor::TypeDescriptor(size_t id, const string& name)
        : base_(0), id_(id), name_(name) {
}


/*
 * ___________________________________________________________________________
 */
TypeDescriptor::TypeDescriptor(const TypeDescriptor& base,
        size_t id, const string& name)
        : base_(&base), id_(id), name_(name) {
}


/*
 * ___________________________________________________________________________
 */
bool TypeDescriptor::inheritsFrom(const TypeDescriptor& other) const {

    bool inherits = false;

    const TypeDescriptor* type = this->getBaseTypeDescriptor();
    while (type != 0 && !inherits) {
        if (*type == other) {
            inherits = true;
        } else {
            type = type->getBaseTypeDescriptor();
        }
    }

    return inherits;
}


/*
 * ___________________________________________________________________________
 */
string TypeDescriptor::getTypeInheritance() const {

    string inheritance;

    const TypeDescriptor* type = this;
    while (type != 0) {
        if (inheritance.length() > 0) {
            inheritance.insert(0, 1, '/');
        }
        inheritance.insert(0, type->name());
        type = type->getBaseTypeDescriptor();
    }

    return inheritance;
}


/*
 * ___________________________________________________________________________
 */
TypeDescriptor::~TypeDescriptor() {
}


/*
 * ===========================================================================
 */


/*
 * ___________________________________________________________________________
 */
TypeDescriptor DataUnit::desc_(0, "DataUnit");


/*
 * ___________________________________________________________________________
 */
DataUnit::DataUnit() : dissector_(*this), parent_(0), previous_(0), next_(0) {
}


/*
 * ___________________________________________________________________________
 */
const TypeDescriptor& DataUnit::getTypeDescriptor() const {

    return typeDescriptor();
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::newInstance_() const {

    /* DataUnit is an abstract class */
    return (DataUnit*)0;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::newInstance() const {

    DataUnit* newInst = this->newInstance_();
    if (newInst != 0) {
        newInst->setName(name_);
    }
    return newInst;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::clone() const {

    DataUnit* newInst = this->newInstance();
    if (newInst != 0) {
        newInst->dissector().dissectFromBuffer(*this);
    }
    return newInst;
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getRefName() const {

    String name = this->getName();
    size_t suffix = 0;

    const DataUnit* du = this->getPrevious();
    while (du != 0) {
        if (name.empty() || du->getName() == name) {
            ++suffix;
        }
        du = du->getPrevious();
    }

    if (suffix > 0 || name.empty()) {
        name.appendFormat("~%u", suffix);
    }

    return name;
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getIdentifier(bool name, bool staticType,
        bool dynamicType) const {

    string identifier;

    if (staticType) {
        identifier.append(this->getStaticType());
    }
    if (dynamicType) {
        string dynamicTypeStr = this->getDynamicType();
        if (!dynamicTypeStr.empty()) {
            identifier.append(":");
            identifier.append(dynamicTypeStr);
        } else {
            dynamicType = false;
        }
    }
    if (staticType || dynamicType) {
        identifier.append("%");    
    }
    if (name) {
        identifier.append(this->getName());
    }

    return identifier;
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getUniqueIdentifier() const {
    
    string name = this->getName();
    string staticType = this->getStaticType();
    string dynamicType = this->getDynamicType();

    size_t indexByName = 0;
    size_t indexByType = 0;
    size_t indexByFullType = 0;
    size_t indexByFullTypeAndName = 0;

    const DataUnit* du = this->getPrevious();
    while (du != 0) {
        if (du->matchesFilter(name)) {
            ++indexByName;
        }
        if (du->matchesFilter("", staticType, "")) {
            ++indexByType;
        }
        if (du->matchesFilter("", staticType, dynamicType)) {
            ++indexByFullType;
        }
        if (du->matchesFilter(name, staticType, dynamicType)) {
            ++indexByFullTypeAndName;
        }
        du = du->getPrevious();
    }

    String id;
    size_t index = 0;

    if (indexByFullTypeAndName == 0 && indexByFullType > 0) {
        id = this->getIdentifier(true, true, true);
        index = indexByFullTypeAndName;

    } else if (indexByFullType == 0 && indexByType > 0) {
        id = this->getIdentifier(false, true, true);
        index = indexByFullType;

    } else if (indexByType == 0 && indexByName > 0) {
        id = this->getIdentifier(false, true, false);
        index = indexByType;

    } else {
        id = name;
        index = indexByName;
    }

    if (index > 0) {
        id.appendFormat("~%u", index);
    }

    return id;
}


/*
 * ___________________________________________________________________________
 */
bool DataUnit::splitFilterExpression(const string& filterExpr, string& name,
        string& staticType, string& dynamicType) {

    // Extract name
    size_t namePos = filterExpr.find_first_of("%");
    if (namePos != string::npos) {
        name = filterExpr.substr(namePos + 1);
    } else {
        // >>> It's a name-only filter expression >>>
        name = filterExpr;
        return true;
    }

    // Extract type
    size_t typePos = filterExpr.find_first_of(":");
    if (typePos != string::npos) {
        if (typePos > namePos) {
            // >>> Invalid filter expression >>>
            return false;
        }
        staticType = filterExpr.substr(0, typePos);
        dynamicType = filterExpr.substr(typePos + 1, namePos - typePos - 1);
    } else {
        // It's a static-type-only type filter expression
        staticType = filterExpr.substr(0, namePos);
    }

    return true;
}


/*
 * ___________________________________________________________________________
 */
bool DataUnit::matchesFilter(const string& filterExpr) const {

    string name;
    string staticType;
    string dynamicType;

    if (!splitFilterExpression(filterExpr, name, staticType, dynamicType)) {
        return false;
    }

    return this->matchesFilter(name, staticType, dynamicType);
}


/*
 * ___________________________________________________________________________
 */
bool DataUnit::matchesFilter(const string& name,
        const string& staticType, const string& dynamicType) const {

    return
        // Matching name
        (name.empty() || name == this->getName()) &&
        // Matching static type
        (staticType.empty() || staticType == this->getStaticType()) &&
        // Matching dynamic type
        (dynamicType.empty() || dynamicType == this->getDynamicType());
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getChainedName(const string& separator) const {

    string name;

    const DataUnit* du = this;
    while (du != 0) {
        if (name.length() > 0) {
            name.insert(0, separator);
        }
        name.insert(0, du->getName());
        du = du->getParent();
    }

    return name;
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getChainedRefName(const string& separator) const {

    string name;

    const DataUnit* du = this;
    while (du != 0) {
        if (name.length() > 0) {
            name.insert(0, separator);
        }
        name.insert(0, du->getRefName());
        du = du->getParent();
    }

    return name;
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getChainedIndex(const string& separator) const {

    string name;

    const DataUnit* du = this;
    while (du != 0) {
        if (name.length() > 0) {
            name.insert(0, separator);
        }
        name.insert(0, String::format("%u", du->getIndexOffset()));
        du = du->getParent();
    }

    return name;
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getPath() const {

    string path;

    const DataUnit* du = this;
    while (du != 0) {
        path.insert(0, du->getRefName());
        path.insert(0, 1, '/');
        du = du->getParent();
    }

    // Finally insert the leading slash
    path.insert(0, 1, '/');

    return path;
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getFullType() const {

    string fullType = this->getStaticType();

    string dynamicType = this->getDynamicType();
    if (!dynamicType.empty()) {
        // >>> This node has a dynamic type (that is,
        // a non-empty dynamic type string) >>>
        fullType.append(":");
        fullType.append(dynamicType);
    }

    return fullType;
}


/*
 * ___________________________________________________________________________
 */
bool DataUnit::propRedirectHook_(string& name, PropertyNode*& node) const {

    /*
     *     <tag name>[@[<path>]]
     */

    /* true if there is a redirection */
    bool redirected = false;

    /* the path for redirection */
    string path;

    /* extract path and update name */
    size_t pos = name.find_first_of("@");
    if (pos != string::npos) {
        path = name.substr(pos + 1);
        name.erase(pos);
    }

    if (!path.empty()) {
        /* redirect property access */
        /* TODO: find some way to avoid this const_cast */
        DataUnit* probe = const_cast<DataUnit*>(this)->getByPath(path);
        if (probe != this) {
            node = probe;
            redirected = true;
        }
    }

    return redirected;
}


/*
 * ___________________________________________________________________________
 */
void DataUnit::propReadHook_(const string& name, DynamicValue& value) const {

    if (name == ".typename") {
        value.setValue<string>(this->getStaticType());
    } else if (name == ".overflow") {
        // Overflow discarded by dissector?
        bool hasOverflow = this->dissector().hasOverflow();
        // Overflow pushed into additional overflow node?
        DataUnit* ovNode = this->getChildTail();
        if (ovNode != 0 && ovNode->getName() == ".overflow"
                && ovNode->getLength() > 0) {
            hasOverflow = true;
        }
        value.setValue<bool>(hasOverflow);
    } else if (name == ".underflow") {
        value.setValue<bool>(this->dissector().hasUnderflow());
    }
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getAnchorBrackets_() const {

    return "[]";
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getChild_() const {

    /* The DataUnit class does not have children */
    return (DataUnit*)0;
}


/*
 * ___________________________________________________________________________
 */
void DataUnit::setChild_(DataUnit* child) {
}


/*
 * ___________________________________________________________________________
 */
bool DataUnit::setChild(DataUnit* child) {

    bool success = false;
    if (child != 0 && child->isRoot()) {
        this->setChild_(child);
        child->setParent(this);
        success = true;
    } else if (child == 0) {
        this->setChild_(child);
        success = true;
    }
    return success;
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getAnchorString_() const {

    string brackets = this->getAnchorBrackets_();
    if (brackets.length() != 2) {
        brackets = "[]";
    }

    String anchor;

    if (this->dissector().isAccepting()) {
        anchor.append(">>");
    } else {
        anchor.append("--");
    }

    if (this->propGetDefault<bool>(".overflow", false)) {
        anchor.appendBoldRedBg("<OF>");
    } else if (this->dissector().hasUnderflow()) {
        anchor.appendBoldRedBg("<UF>");
    }

    anchor.append(1, brackets[0]);

    anchor.appendFormat("%d:", this->getIndexOffset());

    if (this->isDecoded()) {
        anchor.appendBoldGreen(this->getName());
    } else {
        anchor.appendBoldBlue(this->getName());
    }

    anchor.append(1, brackets[1]);
    /*
    String dissector;
    String strSize = this->getDissectorSize().toStringDefault("-");
    String strLimit = this->getDissectorLimit().toStringDefault("-");
    String strAccept = this->getDissectorAcceptance().toStringDefault("*");
    dissector.appendFormat(" [%s:%s:%s]",
            strSize.c_str(), strLimit.c_str(), strAccept.c_str());
    anchor.appendYellow(dissector);
    */
    return anchor;
}


/*
 * ___________________________________________________________________________
 */
BC DataUnit::redissect() {

    /* make a copy of the raw content ... */
    VectorBuffer buffer;
    buffer.appendFromBuffer(*this);

    /* and redissect it */
    this->dissector().reset();
    return this->dissector().dissectFromBuffer(buffer);
}


/*
 * ___________________________________________________________________________
 */
void DataUnit::print() const {

    PrintOptions options;
    this->print(options);
}


/*
 * ___________________________________________________________________________
 */
void DataUnit::print(const PrintOptions& options) const {

    PrintOptions myOptions(options);

    if (!myOptions.bodyOnly) {
        String line(myOptions.prefix);
        line.append(this->getAnchorString_());
        line.setWidth(50);
        line.appendFixedWidth(String::format("%s",
                this->getFullType().c_str()), 40);
//                this->getStaticType().c_str(), this->getTypeID()), 40);
//                this->getPath().c_str(), this->getTypeID()), 40);
        cout << line;
    }

    myOptions.bodyOnly = false;

    this->printBody(myOptions);
}


/*
 * ___________________________________________________________________________
 */
void DataUnit::printChain(ssize_t maxLen, const string& prefix) const {

    const DataUnit* unit = this;
    while (unit != 0 && (maxLen != 0)) {
        PrintOptions options;
        options.prefix = prefix;
        unit->print(options);
        unit = unit->getNext();
        if (maxLen > 0) {
            maxLen--;
        }
    }
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getAsText() const {

    return this->getInfo_();
}


/*
 * ___________________________________________________________________________
 */
void DataUnit::printBody(const PrintOptions& options) const {

    cout << " " << this->getInfo_() << endl;
}


/*
 * ___________________________________________________________________________
 */
bool DataUnit::appendNext(DataUnit* next) {

    bool success = false;

    if (!this->hasNext()) {
        if (next != 0 && next->isRoot()) {
            this->setNext(next);
            next->setParent(this->getParent());
            next->setPrevious(this);
            success = true;
        }
    } else {
        /* select the last child in the linked
         * list and append new element there */
        success = this->getTail()->appendNext(next);
    }

    return success;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::remove() {

    if (previous_ != 0) {
        previous_->setNext(next_);
    } else if (parent_ != 0) {
        parent_->setChild_(next_);
    }
    if (next_ != 0) {
        next_->setPrevious(previous_);
    }

    /* disconnect this data unit from surrounding data units */
    previous_ = (DataUnit*)0;
    next_ = (DataUnit*)0;
    parent_ = (DataUnit*)0;

    return this;
}


/*
 * ___________________________________________________________________________
 */
bool DataUnit::replaceBy(DataUnit* replacement) {

    bool success = false;

    if (replacement != 0 && replacement != this &&
            replacement->isRoot() && !replacement->hasNext()) {

        /* set pointers from surrounding data units to replacement */
        if (previous_ != 0) {
            previous_->setNext(replacement);
        } else if (parent_ != 0) {
            parent_->setChild_(replacement);
        }
        if (next_ != 0) {
            next_->setPrevious(replacement);
        }

        /* set pointers from replacement to surrounding data units */
        replacement->setNext(next_);
        replacement->setPrevious(previous_);
        replacement->setParent(parent_);

        /* disconnect this data unit from surrounding data units */
        previous_ = (DataUnit*)0;
        next_ = (DataUnit*)0;
        parent_ = (DataUnit*)0;

        success = true;
    }

    return success;
}


/*
 * ___________________________________________________________________________
 */
bool DataUnit::insert(DataUnit* insertion) {

    bool success = false;

    if (insertion != 0 && insertion != this &&
            insertion->isRoot() && !insertion->hasNext()) {

        /* set pointers from previous/parent data units to insertion */
        if (previous_ != 0) {
            previous_->setNext(insertion);
        } else if (parent_ != 0) {
            parent_->setChild(insertion);
        }

        /* set pointers from insertion to surrounding data units */
        insertion->setNext(this);
        insertion->setPrevious(previous_);
        insertion->setParent(parent_);

        /* connect this data unit to insertion */
        previous_ = insertion;

        success = true;
    }

    return success;
}


/*
 * ___________________________________________________________________________
 */
size_t DataUnit::deleteFollowing() {

    size_t n = 0;
    DataUnit* unit = next_;

    /* now, the chain stops here */
    next_ = (DataUnit*)0;

    while (unit != 0) {
        DataUnit* next = unit->getNext();
        delete unit;
        n++;
        unit = next;
    }

    return n;
}


/*
 * ___________________________________________________________________________
 */
size_t DataUnit::getIndexOffset() const {

    size_t index = 0;
    DataUnit* unit = this->getPrevious();
    while (unit != 0) {
        index++;
        unit = unit->getPrevious();
    }
    return index;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getRoot() {

    DataUnit* root    = this;
    DataUnit* hop    = this;
    while (hop != 0) {
        /* Hop to root without using recursion */
        root = hop;
        hop = hop->getParent();
    }
    return root;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getHead() {

    DataUnit* head    = this;
    DataUnit* hop    = this;
    while (hop != 0) {
        /* Hop to head (beginning of chain) without using recursion */
        head = hop;
        hop = hop->getPrevious();
    }
    return head;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getTail() {

    DataUnit* tail    = this;
    DataUnit* hop    = this;
    while (hop != 0) {
        /* Hop to tail (end of chain) without using recursion */
        tail = hop;
        hop = hop->getNext();
    }
    return tail;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getPredecessor() const {

    /*
     * getPredecessor() can be defined as 'const' because it can never return
     * a pointer to the callee node ('this').
     */

    DataUnit* predecessor = (DataUnit*)0;

    if (this->hasPrevious()) {
        DataUnit* du = this->getPrevious();
        while (du != 0) {
            predecessor = du;
            du = du->getChildTail();
        }
    } else if (this->hasParent()) {
        predecessor = this->getParent();
    }

    return predecessor;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getSuccessor() const {

    /*
     * getPredecessor() can be defined as 'const' because it can never return
     * a pointer to the callee node ('this').
     */

    DataUnit* successor = (DataUnit*)0;

    if (this->hasChild()) {
        successor = this->getChild_();
    } else if (this->hasNext()) {
        successor = this->getNext();
    } else {
        DataUnit* parent = this->getParent();
        while (parent != 0 && successor == 0) {
            successor = parent->getNext();
            parent = parent->getParent();
        }
    }

    return successor;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getPrevious(const string& filterExpr) const {

    string name;
    string staticType;
    string dynamicType;

    if (!splitFilterExpression(filterExpr, name, staticType, dynamicType)) {
        return (DataUnit*)0;
    }

    return this->getPrevious(name, staticType, dynamicType);
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getPrevious(const string& name,
            const string& staticType, const string& dynamicType) const {

    DataUnit* previous = this->getPrevious();
    while (previous != 0) {
        if (previous->matchesFilter(name, staticType, dynamicType)) {
            break;
        }
        previous = previous->getPrevious();
    }
    return previous;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getNext(const string& filterExpr) const {

    string name;
    string staticType;
    string dynamicType;

    if (!splitFilterExpression(filterExpr, name, staticType, dynamicType)) {
        return (DataUnit*)0;
    }

    return this->getNext(name, staticType, dynamicType);
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getNext(const string& name,
            const string& staticType, const string& dynamicType) const {

    DataUnit* next = this->getNext();
    while (next != 0) {
        if (next->matchesFilter(name, staticType, dynamicType)) {
            break;
        }
        next = next->getNext();
    }
    return next;
}


/*
 * ___________________________________________________________________________
 */
/* TODO: rename to "getNeighborByRelIndexConst" */
DataUnit* DataUnit::getByRelIndexConst(int index) const {

    bool stop = false;
    DataUnit* unit = (DataUnit*)0;

    while (index != 0 && !stop) {
        if (index > 0) {
            unit = (unit != 0 ? unit : this)->getNext();
            index--;
        } else {
            unit = (unit != 0 ? unit : this)->getPrevious();
            index++;
        }
        stop = (unit == 0);
    }

    return unit;
}


/*
 * ___________________________________________________________________________
 */
/* TODO: rename to "getNeighborByRelBC" */
DataUnit* DataUnit::getByRelBC(BC& bc) {

    bool stop = false;
    DataUnit* unit = this;

    while (unit != 0 && !stop) {
        BC len = unit->getLength();
        if (bc >= len) {
            bc -= len;
            unit = unit->getNext();
        } else {
            stop = true;
        }
    }

    return unit;
}


/*
 * ___________________________________________________________________________
 */
/* TODO: rename to "getNeighborByConstRelBC" */
DataUnit* DataUnit::getByConstBC(const BC& bc) {

    BC bc_ = bc;
    return this->getByRelBC(bc_);
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getByPath(const string& path) {

    /*  
     *  Special names:
     *      "."     Current node
     *      ".."    Parent node
     *      "*"     One arbitrary node
     *      "**"    One or more arbitrary nodes
     */

    bool vRoot = false;
    DataUnit* du = this;
    size_t len = path.length();

    size_t pos = path.find_first_not_of("/");
    if (pos != string::npos) {
        if (pos >= 1) {
            // >>> There are leading slashes >>>
            du = this->getRoot();
        }
        if (pos > 1) {
            // Start from virtual root whenever
            // there is more than one leading slash
            vRoot = true;
        }
    }

    string primaryRef;
    while (du != 0 && pos != string::npos && pos < len) {

        // The input path is split according to the following scheme:
        //
        //   [/[/]]PRIMARY-REFERENCE[/SECONDARY-PATH]
        //
        // Examples are:
        //
        //   "primary-reference"
        //   "/primary-reference"
        //   "//primary-reference"
        //   "primary-reference/secondary/path"
        //   "/primary-reference/secondary/path"
        //   "//primary-reference/secondary/path"

        // Find the first slash *after* the primary reference
        size_t pos2 = path.find_first_of("/", pos);

        if (pos2 != string::npos) {
            primaryRef = path.substr(pos, pos2 - pos);
            pos2 = path.find_first_not_of("/", pos2);
        } else {
            primaryRef = path.substr(pos);
            pos2 = len;
        }

        // Check for typed wildcard
        bool isTypedWildcard = false;
        size_t indexPos = primaryRef.find_first_of("~");
        if (indexPos != string::npos) {
            string indexStr = primaryRef.substr(indexPos);
            isTypedWildcard = (indexStr == "~*");
            if (isTypedWildcard) {
                primaryRef.erase(indexPos);
            }
        }

        // Check for any wildcard
        bool isWildcard = isTypedWildcard || (!isTypedWildcard &&
                (primaryRef == "*" || primaryRef == "**"));

        if (primaryRef == "..") {
            if (vRoot) {
                /* A virtual root does not have a parent */
                du = (DataUnit*)0;
            } else if (du->hasParent()) {
                du = du->getParent();
                vRoot = false;
            } else {
                // >>> We are at the root of the GMT >>>
                vRoot = true;
            }

        } else if (isWildcard) {

            string secondaryPath = path.substr(pos2);

            if (vRoot) {
                // >>> We're at the virtual root >>>

                if (primaryRef == "**") {
                    if (!secondaryPath.empty()) {
                        // <du> currently points to root node
                        DataUnit* root = du;
                        // First: try to absorb "**"
                        du = root->getByPath(secondaryPath);
                        // Second: if absorbing "**" did not succeed ...
                        if (du == 0) {
                            // ... keep "**"
                            du = root->getByPath(path.substr(pos));
                        }
                        // The following assignment exhausts remaining iterations
                        pos2 = len;
                    }
                } else if (primaryRef != "*" && !primaryRef.empty()) {
                    // Typed wildcards to not match virtual root
                    du = (DataUnit*)0;
                }

                // We left the virtual root in all cases
                vRoot = false;

            } else {            

                DataUnit* head = du->getChild();
                du = (DataUnit*)0;

                // The following recursion exhausts remaining iterations
                pos2 = len;

                bool second = false;
                do {

                    /*  1st iteration: absorb "*" or "**" and try to match rest
                     *  2nd iteration: keep "**" and try to find a match
                     */

                    DataUnit* child = head;
                    while (child != 0 && du == 0) {
                        // Fast-forward to node matching typed wildcard
                        while (isTypedWildcard && child != 0 &&
                                !child->matchesFilter(primaryRef)) {
                            child = child->getNext();            
                        }
                        if (child != 0 && !secondaryPath.empty()) {
                            du = child->getByPath(secondaryPath);
                            child = child->getNext();
                        } else {
                            // The while loop will terminate in all cases
                            du = child;
                        }
                    }

                    if (second) {
                        // >>> Second iteration done >>>
                        break;
                    }
                    second = true;
                    // For the second iteration we keep the "**/"
                    secondaryPath = path.substr(pos);

                } while (primaryRef == "**" && du == 0);
            }

        } else if (primaryRef != ".") {
            if (vRoot) {
                du = du->getSibling(primaryRef);
                vRoot = false;
            } else {
                du = du->getChildByName(primaryRef);
            }
        }

        pos = pos2;
    }

    if (vRoot) {
        // Cannot return a pointer to the virtual root
        du = (DataUnit*)0;
    }

    return du;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getChild(size_t index) const {

    DataUnit* child = this->getChild_();
    if (child != 0 && index != 0) {
        child = child->getByRelIndex(index);
    }
    return child;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getChildTail() const {

    DataUnit* tail = this->getChild_();
    if (tail != 0) {
        tail = tail->getTail();
    }
    return tail;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getChildByBC(BC& bc) const {

    DataUnit* child = this->getChild_();
    if (child != 0) {
        child = child->getByRelBC(bc);
    }
    return child;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getChildByConstBC(const BC& bc) const {

    DataUnit* child = this->getChild_();
    if (child != 0) {
        child = child->getByConstBC(bc);
    }
    return child;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getSibling(const string& indexedFilterExpr) {

    // Don't accept empty indexed filter expression input
    if (indexedFilterExpr.empty()) {
        return (DataUnit*)0;
    }

    // True if the filter expression has to match exactly one node
    bool unique = false;

    // True if the search is to be performed backwards
    bool backwards = false;

    // True if indexing is relative to 'this' node
    bool relative = false;

    // The absolute index (note that, internally, counting starts at 1)
    int index = 1;

    // Filter expression without index
    string filterExpr(indexedFilterExpr);

    // Split off and parse index 
    size_t pos = indexedFilterExpr.find_first_of("~");
    if (pos != string::npos) {
        // >>> Filter expression is indexed (i.e., has an index) >>>

        // Check for uniqueness request (index == "!")
        if (pos == indexedFilterExpr.length() - 2
                && indexedFilterExpr[pos + 1] == '!') {

            unique = true;

        } else {

            size_t numOffset = 0;

            // Check for relative indexing
            if (pos < indexedFilterExpr.length() - 1
                    && indexedFilterExpr[pos + 1] == 'r') {
                // >>> Indexing is relative to  >>>
                relative = true;
                numOffset += 1;
            }

            // Check for negative index
            if ((pos + numOffset) < indexedFilterExpr.length() - 1 
                    && indexedFilterExpr[pos + numOffset + 1] == '-') {
                // >>> Index is negative >>>
                backwards = true;
                numOffset += 1;
            }

            // Sanitize index: must be a decimal number and nothing else
            if ((pos + numOffset) == indexedFilterExpr.length() - 1 ||
                    indexedFilterExpr.find_first_not_of("0123456789",
                            pos + numOffset + 1) != string::npos) {
                // >>> Invalid index >>>
                return (DataUnit*)0;
            }

            // Extract index as positive number and strip off from filter
            // expression
            filterExpr.erase(pos);
            index = atoi(indexedFilterExpr.substr(pos + numOffset + 1).c_str());
            if (!backwards || relative) {
                index += 1;
            }
        }
    }

    // Split filter expression into its components
    string name;
    string staticType;
    string dynamicType;
    if (!splitFilterExpression(filterExpr, name, staticType, dynamicType)) {
        // >>> Invalid filter expression >>>
        return (DataUnit*)0;
    }

    // Will become a pointer to the requested sibling node
    DataUnit* sibling = (DataUnit*)0;

    // Where to start the search?
    DataUnit* node = this;
    if (!relative) {
        if (backwards) {
            // Start at the tail of the sibling chain
            node = this->getTail();
        } else {
            // Start at the head of the sibling chain
            node = this->getHead();
        }
    }

    // Do the search by hopping from node to node
    while (node != 0 && index > 0 && sibling == 0) {
        if (!node-> matchesFilter(name, staticType, dynamicType)
                || --index > 0) {
            // >>> Not yet done (either the node does not match the filter
            // expression or the index has (still) not reached zero >>>
            if (backwards) {
                node = node->getPrevious();
            } else {
                node = node->getNext();
            }
        } else {
            // <node> holds a pointer to the node we were looking for
            sibling = node;
        }
    }

    return sibling;
}


/*
 * ___________________________________________________________________________
 */
DataUnit* DataUnit::getChildByName(const string& name) {

    DataUnit* child = this->getChild_();
    if (child != 0) {
        child = child->getSibling(name);
    }
    return child;
}


/*
 * ___________________________________________________________________________
 */
size_t DataUnit::getNElements() const {

    size_t n = this->getIndexOffset() + 1;
    DataUnit* unit = this->getNext();
    while (unit != 0) {
        ++n;
        unit = unit->getNext();
    }
    return n;
}


/*
 * ___________________________________________________________________________
 */
size_t DataUnit::getNChildren() const {

    size_t n = 0;
    DataUnit* child = this->getChild_();
    if (child != 0) {
        n = child->getNElements();
    }
    return n;
}


/*
 * ___________________________________________________________________________
 */
size_t DataUnit::getNNodes() const {

    /* this node counts as one */
    size_t n = 1;

    /* sum the number of nodes from children */
    DataUnit* child = this->getChild_();
    while (child != 0) {
        n += child->getNNodes();
        child = child->getNext();
    }

    return n;
}


/*
 * ___________________________________________________________________________
 */
void DataUnit::reset() {

    this->dissector().reset();
    this->reset_();
}


/*
 * ___________________________________________________________________________
 */
void DataUnit::reset_() {
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getInfo_() const {

    return "";
}


/*
 * ___________________________________________________________________________
 */
bool DataUnit::repair_(bool recursive) {

    return true;
}


/*
 * ___________________________________________________________________________
 */
string DataUnit::getDynamicType_() const {

    return "";
}


/*
 * ___________________________________________________________________________
 */
bool DataUnit::isDescendantOf(const DataUnit& dataUnit) const {

    bool isDescendant = false;

    const DataUnit* parent = this->getParent();
    while (!isDescendant && parent != 0) {
        if (parent == &dataUnit) {
            isDescendant = true;
        } else {
            parent = parent->getParent();
        }
    }

    return isDescendant;
}


/*
 * ___________________________________________________________________________
 */
size_t DataUnit::getDistanceToRoot() const {

    size_t dist = 0;

    const DataUnit* du = this;
    while (du != 0) {
        ++dist;
        du = du->getParent();
    }

    /* we always count one too much */
    return dist - 1;
}


/*
 * ___________________________________________________________________________
 */
BC DataUnit::getDataOffset() const {

    BC offset = 0;

    DataUnit* unit = this->getPrevious();
    while (unit != 0) {
        offset += unit->getLength();
        unit = unit->getPrevious();
    }

    return offset;
}


/*
 * ___________________________________________________________________________
 */
bool DataUnit::checkConsistency(bool printErrors) const {

    bool consistent = true;

    if (this->hasNext()) {

        DataUnit* next = this->getNext();
        /* the previous element of the next element needs to be this one */
        if (next->getPrevious() != this) {
            consistent = false;
            if (printErrors) {
                cout << String::format("Broken right back-reference of '%s'\n",
                        this->getChainedRefName().c_str());
            }
        }

        /* the parent of the next element needs to be the parent of this one */
        if (consistent && next->getParent() != this->getParent()) {
            consistent = false;
            if (printErrors) {
                cout << String::format("Mismatching parents in '%s'\n",
                        this->getChainedRefName().c_str());
            }
        }

        /* the next elements needs to be consistent */
        consistent &= next->checkConsistency();
    }

    DataUnit* child = this->getChild();
    if (consistent && child != 0) {

        /* the previous element of the child element needs to be non-existent */
        if (child->hasPrevious()) {
            consistent = false;
            if (printErrors) {
                cout << String::format("Backward reference of child of '%s'\n",
                        this->getChainedRefName().c_str());
            }
        }

        /* the parent of the child element needs to be this one */
        if (consistent && child->getParent() != this) {
            consistent = false;
            if (printErrors) {
                cout << String::format(
                        "Broken parent reference from '%s' to '%s'\n",
                        child->getChainedRefName().c_str(),
                        this->getChainedRefName().c_str());
            }
        }

        /* the child element needs to be consistent */
        consistent &= child->checkConsistency();
    }

    return consistent;
}


/*
 * ___________________________________________________________________________
 */
DataUnit::~DataUnit() {
}
