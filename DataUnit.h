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

#ifndef __DataUnit_H__
#define __DataUnit_H__

#include <string>
#include <vector>
#include <inttypes.h>
#include "PropertyNode.h"
#include "BufferReader.h"
#include "BufferWriter.h"
#include "BufferModifier.h"
#include "Dissector.h"

class BufferStreamReader;
class TypeDescriptor;


/* ==========================================================================
 *
 *
 * ========================================================================== */

class TypeRegistry {

private:

	// TODO: Add description
	typedef std::vector<const TypeDescriptor*> registry_t;

	// TODO: Add description
	registry_t registry_;

	// TODO: Add description
	static TypeRegistry* singleton;


public:

	// TODO: Add description
	static TypeRegistry& getInstance();


	// TODO: Add description
	TypeRegistry();


	// TODO: Add description
	bool add(const TypeDescriptor& typeDescriptor);

	// TODO: Add description
	const TypeDescriptor* findByName(const std::string& typeName);

	// TODO: Add description
	const TypeDescriptor* findById(size_t typeId);

	// TODO: Add description
	size_t print();


	// TODO: Add description
	~TypeRegistry();
};




/* ==========================================================================
 *
 *
 * ========================================================================== */

class TypeDescriptor {

private:

	// TODO: Add description
	const TypeDescriptor* base_;

	// TODO: Add description
	size_t id_;

	// TODO: Add description
	const std::string name_;


public:

	// TODO: Add description
	TypeDescriptor(size_t id, const std::string& name);

	// TODO: Add description
	TypeDescriptor(const TypeDescriptor& baseTypeDesc,
			size_t id, const std::string& name);


	// TODO: Add description
	inline bool registerType() const {

		return this->registerType(TypeRegistry::getInstance());
	}

	// TODO: Add description
	inline bool registerType(TypeRegistry& registry) const {

		return registry.add(*this);
	}


	// TODO: Add description
	inline size_t id() const {

		return id_;
	}

	// TODO: Add description
	inline const std::string& name() const {

		return name_;
	}

	// TODO: Add description
	inline const TypeDescriptor* getBaseTypeDescriptor() const {

		return base_;
	}


	// TODO: Add description
	inline bool operator==(const TypeDescriptor& desc) const {

		return this == &desc;
	}

	// TODO: Add description
	inline bool operator!=(const TypeDescriptor& desc) const {

		return !(this == &desc);
	}


	// TODO: Add description
	bool inheritsFrom(const TypeDescriptor& other) const;

	// TODO: Add description
	inline bool contains(const TypeDescriptor& other) const {

		return (*this == other) || this->inheritsFrom(other);
	}


	// TODO: Add description
	std::string getTypeInheritance() const;


	// TODO: Add description
	~TypeDescriptor();

};



/* ==========================================================================
 *
 * +-------+                                              +-------+
 * |   1   |<-------------------------------------------->|  [9]  |---...
 * +-------+                                              +-------+
 *     ^
 *     |     +-------+                          +-------+
 *     +---->|   2   |<------------------------>|   7   |---...
 *           +-------+                          +-------+
 *               ^                                  ^
 *               |     +-------+      +-------+     |     +-------+
 *               +---->|  [3]  |<---->|   4   |     +---->|  [8]  |
 *                     +-------+      +-------+           +-------+
 *                                        ^
 *                                        |     +-------+      +-------+
 *                                        +---->|  [5]  |<---->|  [6]  |
 *                                              +-------+      +-------+
 *
 * ========================================================================== */

class DataUnit : public PropertyNode, protected DissectorBody,
		public BufferReader, public BufferWriter, public BufferModifier {

public:

	class PrintOptions {

	public:

		BC			from;
		BC			to;
		std::string	prefix;
		bool		bodyOnly;

		PrintOptions() {
			bodyOnly = false;
		}

	};


private:

	/* --- Static class members -------------------------------------------- */

	/* this class type's descriptor */
	static TypeDescriptor desc_;


	// Splits a filter expression ("[[static_type][:dynamic_type]%][name]") 
    // into its static type, dynamic type, and name components
    static bool splitFilterExpression(const std::string& filterExpr,
            std::string& name, std::string& staticType,
            std::string& dynamicType);


	/* --- Non-static class members ---------------------------------------- */

	/* data unit dissector */
	Dissector dissector_;

	/* Pointer to this data unit's container */
	DataUnit* parent_;

	/* Pointer to data unit preceeding this data unit in the chain */
	DataUnit* previous_;

	/* Pointer to data unit following this data unit in the chain */
	DataUnit* next_;


	// TODO: Add description
	std::string	name_;


	// TODO: Add description
	inline void setPrevious(DataUnit* previous) {

		previous_ = previous;
	}

	// TODO: Add description
	inline void setNext(DataUnit* next) {

		next_ = next;
	}


protected:

	// TODO: Add description
	virtual void printBody(const PrintOptions& options) const;


	/* --- To be implemented by sub-classes: ------------------------------- */

	// TODO: Add description
	virtual bool isDecoded_() const = 0;

	/* --- To be overwritten by sub-classes: ------------------------------- */

	// TODO: Add description
	virtual DataUnit* newInstance_() const;

	// TODO: Add description
	virtual bool repair_(bool recursive = true);

	// TODO: Add description
	virtual std::string getDynamicType_() const;

	/* --- Potentially to be overwritten by sub-classes: ------------------- */

	// TODO: Add description
	virtual bool propRedirectHook_(std::string& name,
			PropertyNode*& node) const;

	// TODO: Add description
    virtual void propReadHook_(const std::string& name,
    		DynamicValue& value) const;


	// TODO: Add description
    virtual void reset_();


	// TODO: Add description
	virtual std::string getInfo_() const;

	// TODO: Add description
	virtual std::string getAnchorBrackets_() const;


	// TODO: Add description
	virtual DataUnit* getChild_() const;

	// TODO: Add description
	virtual void setChild_(DataUnit* child);

	/* --------------------------------------------------------------------- */

	// TODO: Add description
	inline void setParent(DataUnit* parent) {

		parent_ = parent;
	}

	// TODO: Add description
	bool setChild(DataUnit* child);

	// TODO: Add description
	std::string getAnchorString_() const;


public:

	// TODO: Add description
	static inline const TypeDescriptor& typeDescriptor() {

		return desc_;
	}


	/* Default constructor */
	DataUnit();


	// TODO: Add description
	inline Dissector& dissector() {

		return dissector_;
	}

	// TODO: Add description
	inline const Dissector& dissector() const {

		return dissector_;
	}


	// TODO: Add description
	inline PropertyNode& properties() {

		return static_cast<PropertyNode&>(*this);
	}


	// TODO: Add description
	virtual const TypeDescriptor& getTypeDescriptor() const;


	/* Object factory */
	DataUnit* newInstance() const;

	/* Object cloning */
	DataUnit* clone() const;


    /* Reseting state */
    void reset();


	/* Method for setting this data unit's name */
	inline void setName(const std::string& name) {

		name_ = name;
	}


	/* ---------------------------------------------------------------------
	 *  Attributes of GMT nodes (a.k.a. data units)
	 * --------------------------------------------------------------------- */

    /*  GMT nodes (a.k.a. data units) can have several inherent attributes for
     *  identification:
     *
     *  (1) name
     *  (2) static type (read-only at runtime)
     *  (3) dynamic type (read-only at runtime)
     *
     *  Basically, these attributes are strings (with certain restrictions
     *  that apply to the types of characters that can be used). Name (1) and
     *  dynamic type (3) are optional and might be empty (string of length 0).
     *  The static type (2) cannot be empty. 
     *
     *  The name attribute (1) of a node can be changed at runtime. The other
     *  two attributes, that is (2) and(3), are read-only. Accessor methods are:
     *
     *  -> getName()            // Read name attribute
     *  -> setName(name)        // Write name attribute
     *  -> getStaticType()      // Read static type attribute
     *  -> getDynamicType()     // Read dynamic type attribute
     *
     *
     */


	// Return the name
	inline const std::string& getName() const {

		return name_;
	}

	// Return the static type as string
	inline const std::string& getStaticType() const {

		return this->getTypeDescriptor().name();
	}

	// Return the dynamic type as string (might be empty)
	inline std::string getDynamicType() const {

		return this->getDynamicType_();
	}

	// TODO: Add description
	std::string getFullType() const;



	// TODO: Add description
	std::string getIdentifier(bool name = true, bool staticType = true,
            bool dynamicType = true) const;

	// TODO: Add description
	std::string getUniqueIdentifier() const;

	// TODO: Add description
	bool matchesFilter(const std::string& filterExpr) const;

	// TODO: Add description
	bool matchesFilter(const std::string& name,
            const std::string& staticType, const std::string& dynamicType) const;

	/* Method for getting this data unit's name path */
	std::string getRefName() const;

	/* Method for getting this data unit's name path */
	std::string getChainedName(const std::string& separator = "/") const;

	/* Method for getting this data unit's name path */
	std::string getChainedRefName(const std::string& separator = "/") const;

	/* Method for getting this data unit's index path */
	std::string getChainedIndex(const std::string& separator = "/") const;

	/* Method for getting this data unit's name path */
	std::string getPath() const;






	// TODO: Add description
	inline std::string getTypeInheritance() const {

		return this->getTypeDescriptor().getTypeInheritance();
	}

	// TODO: Add description
	inline bool inheritsFromType(const TypeDescriptor& desc) const {

		return this->getTypeDescriptor().inheritsFrom(desc);
	}

	// TODO: Add description
	inline bool isOfType(const TypeDescriptor& desc) const {

		return this->getTypeDescriptor() == desc;
	}

	// TODO: Add description
	inline bool containsType(const TypeDescriptor& desc) const {

		return this->getTypeDescriptor().contains(desc);
	}


	/* ---------------------------------------------------------------------
	 *  Navigating between data units
	 * ---------------------------------------------------------------------
     *  
     *  Conceptually, Generic Message Trees (GMTs) are trees, as the name 
     *  suggests. However, internally, the nodes are linkes in a different way:
     *
     * +-------+                                              +-------+
     * |   1   |<-------------------------------------------->|   9   |---...
     * +-------+                                              +-------+
     *     ^
     *     |
     *     v
     * +-------+                          +-------+
     * |   2   |<------------------------>|   7   |---...
     * +-------+                          +-------+
     *     ^                                  ^
     *     |                                  |
     *     v                                  v
     * +-------+      +-------+           +-------+
     * |   3   |<---->|   4   |           |   8   |
     * +-------+      +-------+           +-------+
     *                    ^
     *                    |
     *                    v
     *                +-------+      +-------+
     *                |   5   |<---->|   6   |
     *                +-------+      +-------+
     *
     *  Here, for instance, node [1] is the root node and nodes [2] and [7] 
     *  the root's child nodes. That is, instead of each node holding a list
     *  of its child nodes, each node just holds a pointer to its first child
     *  node and its parent (if existent). Additionally, each nodes can have a
     *  'previous' node and a 'next' node, building a doubly-linked list of 
     *  sibling nodes. 
     *
     *                                    ^  parent  
     *                                    |
     *                              +------------+
     *              previous <------|  GMT node  |------> next
     *                              +------------+
     *                                    |  
     *                                    v  first child
     *  
     *  Navigating through a GMT can be done in several ways. One way is to 
     *  call methods defined in the DataUnit (GMT node) class:
     *  
     *  -> getParent()
     *
     *  
     *
     *  -> getPrevious()
     *  -> getPrevious(filterExpr)
     *  -> getPrevious(name, staticType, dynamicType)
     *
     *  -> getNext()
     *  -> getNext(filterExpr)
     *  -> getNext(name, staticType, dynamicType)
     *
     *
     *
     *
     *
     *
     *
     *
     *
     *
     *  GMT nodes have three attributes (name, static type, dynamic type) that
     *  can be used to define a filter on GMT nodes using the following filter
     *  expression syntax
     *
     *      "[[static_type][:dynamic_type]%][name]"    (filter expression)
     *
     *  It matches a node if the the considered node's attributes are identical
     *  to those given in the filter expression. If an attribute is omitted in
     *  the filter expression it essentially acts as a wildcard. Examples are:
     *
     *  -> "length"                     (selects nodes that are named "length")
     *  -> "uint32%"                    (selects nodes of static type "uint32")
     *  -> "TLSExtension:heartbeat%"    (selects nodes of static type
     *                                   "TLSExtension" with a dynamic type
     *                                   "heartbeat")
     *
     *  
     *
     *
     */

	/* Return pointer to this data unit's container */
	inline DataUnit* getParent() const {

		return parent_;
	}

    
	/* --- getPrevious(...) ------------------------------------------------ */

	/* Return pointer to data unit preceeding this data unit in the chain */
	inline DataUnit* getPrevious() const {

		return previous_;
	}

	/* TODO: Add description */
	DataUnit* getPrevious(const std::string& filterExpr) const;

	/* TODO: Add description */
	DataUnit* getPrevious(const std::string& name,
            const std::string& staticType, const std::string& dynamicType) const;

    
	/* --- getNext(...) ---------------------------------------------------- */


	/* Return pointer to data unit following this data unit in the chain */
	inline DataUnit* getNext() const {

		return next_;
	}

	/* TODO: Add description */
	DataUnit* getNext(const std::string& filterExpr) const;

	/* TODO: Add description */
	DataUnit* getNext(const std::string& name,
            const std::string& staticType, const std::string& dynamicType) const;




	/* Return pointer to data unit at index <index> in this data unit's chain */
	DataUnit* getByRelIndexConst(int index) const;

	/* Return pointer to data unit at index <index> in this data unit's chain */
	inline DataUnit* getByRelIndex(int index) {

		return (index == 0) ? this : this->getByRelIndexConst(index);
	}

	/* Return pointer to data unit at byte position <pos> in this data unit's chain */
	DataUnit* getByRelBC(BC& bc);

	/* Return pointer to data unit at byte position <pos> in this data unit's chain */
	DataUnit* getByConstBC(const BC& bc);

	/* Return pointer to data unit referred to by relative path <path> */
	DataUnit* getByPath(const std::string& path);


	// TODO: Add description
	inline DataUnit* getChild() const {

		return this->getChild_();
	}

	// TODO: Add description
	DataUnit* getChild(size_t index) const;

	// TODO: Add description
	DataUnit* getChildTail() const;

	// TODO: Add description
	DataUnit* getChildByBC(BC& bc) const;

	// TODO: Add description
	DataUnit* getChildByConstBC(const BC& bc) const;

	// TODO: Add description
	DataUnit* getChildByName(const std::string& name);


	// TODO: Add description
	DataUnit* getNeighbourByName(const std::string& name);


	// TODO: Add description
	DataUnit* getRoot();

	// TODO: Add description
	DataUnit* getHead();

	// TODO: Add description
	DataUnit* getTail();

	// TODO: Add description
	DataUnit* getPredecessor() const;

	// TODO: Add description
	DataUnit* getSuccessor() const;


	// TODO: Add description
	inline bool hasParent() const {

		return (parent_ != 0);
	}

	// TODO: Add description
	inline bool hasPrevious() const {

		return (previous_ != 0);
	}

	// TODO: Add description
	inline bool hasNext() const {

		return (next_ != 0);
	}

	// TODO: Add description
	inline bool hasChild() const {

		return (this->getChild_() != 0);
	}

	// TODO: Add description
	inline bool isRoot() const {

		return (parent_ == 0) && (previous_ == 0);
	}

	// TODO: Add description
	inline bool isPrimary() const {

		return (parent_ == 0) && (previous_ == 0);
	}

	// TODO: Add description
	bool isDescendantOf(const DataUnit& dataUnit) const;


	// TODO: Add description
	size_t getDistanceToRoot() const;


	// TODO: Add description
	inline bool repair(bool recursive = true) {

		return this->repair_(recursive);
	}


	// TODO: Add description
	BC redissect();


	// TODO: Add description
	void print() const;

	// TODO: Add description
	void print(const PrintOptions& options) const;

	// TODO: Add description
	void printChain(ssize_t maxLen = -1, const std::string& prefix = "") const;


	// TODO: Add description
	virtual std::string getAsText() const;


	// TODO: Add description
	bool appendNext(DataUnit* dataUnit);


	// TODO: Add description
	DataUnit* remove();

	// TODO: Add description
	bool replaceBy(DataUnit* replacement);

	// TODO: Add description
	bool insert(DataUnit* insertion);


	// TODO: Add description
	size_t deleteFollowing();


	// TODO: Add description
	size_t getNElements() const;

	// TODO: Add description
	size_t getNChildren() const;

	// TODO: Add description
	size_t getNNodes() const;


	// TODO: Add description
	inline bool isDecoded() const {

		return (dissector_.getSize() == 0) || this->isDecoded_();
	}


	// TODO: Add description
	size_t getIndexOffset() const;

	// TODO: Add description
	BC getDataOffset() const;


	// TODO: Add description
	inline DataUnit* operator[](size_t index) const {

		return this->getChild(index);
	}


	// TODO: Add description
	inline DataUnit& operator()(size_t index) const {

		return *(this->getChild(index));
	}


	// TODO: Add description
	bool checkConsistency(bool printErrors = false) const;


	// TODO: Add description
	virtual ~DataUnit();

};

#endif
