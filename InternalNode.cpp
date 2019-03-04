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

#include <stdexcept>
#include <iostream>
#include "String_.h"
#include "InternalNode.h"
#include "StreamReader.h"
#include "OpaqueField.h"

using std::cout;
using std::endl;
using std::string;
using std::vector;


/*
 * ___________________________________________________________________________
 */
TypeDescriptor InternalNode::desc_(
		DataUnit::typeDescriptor(), 0, "InternalNode");


/*
 * ___________________________________________________________________________
 */
InternalNode::InternalNode()
		: child_(0), autoExtend_(false), expanded_(false),
		  stalled_(false), dissectionHeadIndex_(0) {
}


/*
 * ___________________________________________________________________________
 */
InternalNode::InternalNode(bool expanded)
		: child_(0), autoExtend_(false), expanded_(expanded),
		  stalled_(false), dissectionHeadIndex_(0) {
}


/*
 * ___________________________________________________________________________
 */
const TypeDescriptor& InternalNode::getTypeDescriptor() const {

	return typeDescriptor();
}


/*
 * ___________________________________________________________________________
 */
string InternalNode::getAnchorBrackets_() const {

	return "{}";
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::printCollapsed_() const {

	return false;
}


/*
 * ___________________________________________________________________________
 */
void InternalNode::dissectorBodyReset() {

	this->deleteChildren();
	expanded_ = false;
	dissectionHeadIndex_ = 0;
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::dissectorBodyIsAccepting() const {

	bool accepting = false;

	if (!expanded_) {
		accepting = !stalled_;
	} else {
		DataUnit* child = child_;
		while (!accepting && child != 0) {
			accepting = child->dissector().isAccepting();
			child = child->getNext();
		}
	}

	return accepting;
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::expand(bool dry) {

	if (!expanded_) {

		/* TODO: make this more efficient */
		size_t nChildren = this->getNChildren();
		size_t nDecoded = this->getNDecoded();

		expanded_ = this->expand_(nChildren, nDecoded, dry,
				dissectionHeadIndex_ < nChildren);
		stalled_ = (dissectionHeadIndex_ > nDecoded);
}

	return expanded_;
}


/*
 * ___________________________________________________________________________
 */
BC InternalNode::dissectorBodyDissect(StreamReader& reader) {

	/* try to expand children (if not yet fully expanded) */
	this->expand(reader.isEmpty());

	BC len = 0;
	DataUnit* unit = this->getChild(dissectionHeadIndex_);

	while (reader.getLength() > 0 && unit != 0) {

		/* feed dissection head with data to dissect */
		if (unit->dissector().isAccepting()) {
			len += unit->dissector().dissect(reader);
		}

		bool gotoNext = !unit->dissector().isAccepting();
		if (gotoNext) {
			dissectionHeadIndex_++;
		}

		/* try to expand children (if not yet fully expanded) */
		this->expand(reader.isEmpty());

		if (gotoNext) {
			unit = unit->getNext();
		}
	}

	BC readerLen = reader.getLength();

	BC acceptance = this->dissector().getAcceptance();
	if (acceptance.isDef() && acceptance > 0) {
		if ((readerLen.isUndef() || readerLen > 0) && autoExtend_) {
			OpaqueField* opaque = new OpaqueField(readerLen);
			len += opaque->dissector().dissect(reader);
			this->appendChildRenamed(opaque, "__overflow");
		}
	}

	return len;
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::repair_(bool recursive) {

	bool repaired = true;

	/* iterate over children from tail to head */
	size_t index = this->getNChildren();
	DataUnit* child = this->getChildTail();
	while (child != 0 && repaired) {
		--index;
		if (recursive) {
			/* step 1 (optional): recursively repair */
			repaired = child->repair(true);
		}
		if (repaired) {
			/* step 2: update this member */
			repaired = this->updateMember(index);
		}
		child = child->getPrevious();
	}

	if ((index == 0) != (child == 0)) {
		/* this should never happen */
		throw std::runtime_error(
				"InternalNode::repair_(...): inconsistency");
	}

	return repaired;
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::getBit_(const BC& bc) const {

	/* InternalNode::getChildByDP(...) will
	 * change its input parameter, so use a copy */
	BC dp_ = bc;

	DataUnit* unit = this->getChildByBC(dp_);

	if (unit != 0) {
		return unit->getBit(dp_);
	} else {
		throw std::runtime_error("InternalNode::getBit(...): Out of range");
	}
}


/*
 * ___________________________________________________________________________
 */
uint8_t	InternalNode::getByte_(const BC& bc) const {

	/* InternalNode::getChildByDP(...) will
	 * change its input parameter, so use a copy */
	BC dp_ = bc;

	DataUnit* unit = this->getChildByBC(dp_);

	if (unit != 0) {

		uint8_t byte = 0;

		BC len = unit->getLength();
		if (dp_ + 1 <= len) {
			byte = unit->getByte(dp_);
		} else {

			for (size_t i = 0; i < 8; i++) {
				if (unit == 0) {
					throw std::runtime_error("InternalNode::"
							"getByte(...): Out of range");
				}
				if (unit->getBit(dp_)) {
					byte += (0x80 >> i);
				}
				dp_ <<= 1;
				if (dp_ >= len && i < 7) {
					unit = unit->getNext();
					if (unit != 0) {
						dp_ -= len;
						len = unit->getLength();
					}
				}
			}
		}

		return byte;

	} else {
		throw std::runtime_error("InternalNode::getByte(...): Out of range");
	}
}


/*
 * ___________________________________________________________________________
 */
BC InternalNode::copyTo_(BufferWriter& buffer) const {

    BC len = 0;

    DataUnit* child = child_;
    while (child != 0) {
        len += child->copyTo(buffer);
        child = child->getNext();
    }

	return len;
}


/*
 * ___________________________________________________________________________
 */
BC InternalNode::getLength_() const {

	BC len = 0;

	DataUnit* unit = child_;
	while (unit != 0) {
		len += unit->getLength();
		unit = unit->getNext();
	}

	return len;
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::appendBit(bool bit) {

	throw std::runtime_error("InternalNode::appendBit(...): "
			"Not yet implemented");
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::appendByte(uint8_t byte) {

	throw std::runtime_error("InternalNode::appendByte(...): "
			"Not yet implemented");
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::setBit(const BC& bc, bool bit) {

	throw std::runtime_error("InternalNode::setBit(...): "
			"Not yet implemented");
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::setByte(const BC& bc, uint8_t byte) {

	throw std::runtime_error("InternalNode::setByte(...): "
			"Not yet implemented");
}


/*
 * ___________________________________________________________________________
 */
void InternalNode::clear() {

	throw std::runtime_error("InternalNode::clear(): "
			"Not yet implemented");
}


/*
 * ___________________________________________________________________________
 */
void InternalNode::zero() {

	DataUnit* child = child_;
    while (child != 0) {
        child->zero();
        child = child->getNext();
    }
}


/*
 * ___________________________________________________________________________
 */
void InternalNode::truncate(const BC& length) {

	BC dp = length;
	DataUnit* unit = this->getChildByBC(dp);

	if (unit != 0) {
		size_t index = unit->getIndexOffset();
		if (dp != 0) {
			unit->truncate(dp);
			++index;
		}
		this->deleteChildren(index);
	}
}


/*
 * ___________________________________________________________________________
 */
string InternalNode::getAsText() const {

	String text;
	text.appendFormat("%s(\"%s\", %s of %s) {",
			this->getStaticType().c_str(), this->getRefName().c_str(),
			this->getLength().toString().c_str(),
			this->dissector().getSize().toString().c_str());

	DataUnit* child = this->getChild();
	if (child != 0) {
		text.append("\n");
	}
	while (child != 0) {

		String line(child->getAsText());
		line.indentLines("    ");

		child = child->getNext();
		line.appendBool(child != 0, ",\n", "\n");

		text.append(line);
	}

	text.append("}");
	return text;
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::isDecoded_() const {

	size_t n = this->getNChildren();
	return (expanded_ && n == this->getNDecoded());
}


/*
 * ___________________________________________________________________________
 */
DataUnit* InternalNode::getChild_() const {

	return child_;
}


/*
 * ___________________________________________________________________________
 */
void InternalNode::setChild_(DataUnit* child) {

	child_ = child;
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::appendChild(DataUnit* child) {

	bool success = false;

	if (child_ == 0) {
		success = this->setChild(child);
	} else {
		success = child_->appendNext(child);
	}

	return success;
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::appendChildRenamed(
		DataUnit* child, const string& newName) {

	bool success = this->appendChild(child);

	if (success && child != 0) {
		child->setName(newName);
	}

	return success;
}


/*
 * ___________________________________________________________________________
 */
void InternalNode::printBody(const PrintOptions& options) const {

	bool prefixMasking = true;
	//bool suffixMasking = true;
	PrintOptions options2(options);

	if (!this->printCollapsed_()) {

		cout << endl;

		DataUnit* child = child_;
		while (child != 0) {

			if (child->hasNext()) {
				options2.prefix = options.prefix + "  |";
			} else {
				options2.prefix = options.prefix + "   ";
			}

			BC length = child->getLength();
			if (options2.from.isUndef() || options2.from < length) {
				child->print(options2);
			} else if (prefixMasking) {
				cout << options2.prefix << "..." << endl;
				prefixMasking = false;
			}

			options2.from -= length;
			options2.to -= length;

			child = child->getNext();
		}
	} else {

		bool first = true;

		String pref(options.prefix);
		pref.setWidth(50);

		VectorBuffer buffer;
		buffer.appendFromBuffer(*this);

		BC pos = 0;
		BC len = buffer.getLength();
		while (pos == 0 || pos < len) {
			String hex = buffer.toHexString(true, pos, 8);
			if (options.from.isDef() && options.from > pos) {
				for (size_t i = 0; i < 8 && i < (options.from - pos).byte(); i++) {
					if (hex.length() > 3*i) {
						hex.at(3*i) = '-';
						hex.at(3*i + 1) = '-';

					}
				}
			}
			hex.setWidth(23);
			if (!first) {
				cout << endl << pref;
			}
			cout << "| " << String::makeBoldGreen(hex) << " |";
			first = false;
			pos += 8;
		}

		cout << endl;
	}
}


/*
 * ___________________________________________________________________________
 */
size_t InternalNode::getNDecoded() const {

	size_t n = 0;
	DataUnit* child = child_;
	while (child != 0) {
		if (child->isDecoded()) {
			n++;
			child = child->getNext();
		} else {
			/* exit loop */
			child = (DataUnit*)0;
		}
	}
	return n;
}

/*
 * ___________________________________________________________________________
 */
bool InternalNode::expand_(
		size_t len, size_t decoded, bool dry, bool ahead) {

	return true;
}


/*
 * ___________________________________________________________________________
 */
bool InternalNode::updateMember_(size_t index) {

	return true;
}


/*
 * ___________________________________________________________________________
 */
size_t InternalNode::deleteChildren(size_t from) {

	size_t n = 0;

	DataUnit* unit = this->getChild((from > 0) ? (from - 1) : 0);
	if (unit != 0) {
		n = unit->deleteFollowing();
	}
	if (from == 0) {
		/* even the first child has to be deleted */
		delete child_;
		child_ = (DataUnit*)0;
		++n;
	}

	return n;
}


/*
 * ___________________________________________________________________________
 */
InternalNode::~InternalNode() {

	this->deleteChildren();
}
