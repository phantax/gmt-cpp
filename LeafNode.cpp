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

#include "LeafNode.h"
#include "String_.h"
#include <iostream>
#include <stdexcept>

using std::cout;
using std::endl;
using std::vector;
using std::string;


/*
 * ___________________________________________________________________________
 */
TypeDescriptor LeafNode::desc_(
		DataUnit::typeDescriptor(), 0, "LeafNode");


/*
 * ___________________________________________________________________________
 */
LeafNode::LeafNode() : DataUnit(), buffer_() {
}


/*
 * ___________________________________________________________________________
 */
LeafNode::LeafNode(const BC& capacity) :
		DataUnit(), buffer_(capacity) {

    /* TODO: do we really need this? */
	this->dissector().setSize(capacity);
}


/*
 * ___________________________________________________________________________
 */
const TypeDescriptor& LeafNode::getTypeDescriptor() const {

	return typeDescriptor();
}


/*
 * ___________________________________________________________________________
 */
void LeafNode::printBody(const PrintOptions& options) const {

	bool first = true;

	String pref(options.prefix);
	pref.setWidth(50);

	BC pos = 0;
	BC len = buffer_.getLength();
	while (pos == 0 || pos < len) {
		String hex = buffer_.toHexString(true, pos, 8);
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
		cout << "| " << String::makeBoldRed(hex) << " |";
		first = false;
		pos += 8;
	}

	DataUnit::printBody(options);
}


/*
 * ___________________________________________________________________________
 */
string LeafNode::getAsText() const {

	String text;
	text.appendFormat("%s(\"%s\", %s of %s) [ %s ] <%s>",
			this->getStaticType().c_str(), this->getRefName().c_str(),
			this->getLength().toString().c_str(),
			this->dissector().getSize().toString().c_str(),
			this->toHexString(true).c_str(),
			this->getInfo_().c_str());
	return text;
}


/*
 * ___________________________________________________________________________
 */
void LeafNode::dissectorBodyReset() {

	/* clear the buffer to accept new data */
	buffer_.clear();
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::dissectorBodyIsAccepting() const {

	BC remCap = buffer_.getRemainingCapacity();
	return remCap.isUndef() || remCap > 0;
}


/*
 * ___________________________________________________________________________
 */
BC LeafNode::dissectorBodyDissect(StreamReader& reader) {

	BC len = buffer_.consumeStream(reader);

	if (buffer_.getCapacity() == buffer_.getLength()) {
		/* trigger decoding once raw data buffer has been filled */
		this->decode_(buffer_);
	}

	return len;
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::encode() {

	return this->encode_();
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::getBit_(const BC& bc) const {

	return buffer_.getBit(bc);
}


/*
 * ___________________________________________________________________________
 */
uint8_t	LeafNode::getByte_(const BC& bc) const {

	return buffer_.getByte(bc);
}


/*
 * ___________________________________________________________________________
 */
BC LeafNode::copyTo_(BufferWriter& buffer) const {

	return buffer_.copyTo(buffer);
}


/*
 * ___________________________________________________________________________
 */
BC LeafNode::getLength_() const {

	return buffer_.getLength();
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::appendBit(bool bit) {

	return buffer_.appendBit(bit);
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::appendByte(uint8_t byte) {

	return buffer_.appendByte(byte);
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::setBit(const BC& bc, bool bit) {

	return buffer_.setBit(bc, bit);
}


/*
 * ___________________________________________________________________________
 */
bool LeafNode::setByte(const BC& bc, uint8_t byte) {

	return buffer_.setByte(bc, byte);
}


/*
 * ___________________________________________________________________________
 */
void LeafNode::clear() {

	buffer_.clear();
}


/*
 * ___________________________________________________________________________
 */
void LeafNode::zero() {

	buffer_.zero();
}


/*
 * ___________________________________________________________________________
 */
void LeafNode::truncate(const BC& length) {

	buffer_.truncate(length);
}


/*
 * ___________________________________________________________________________
 */
void LeafNode::clearBuffer() {

	buffer_.clear();
}


/*
 * ___________________________________________________________________________
 */
BC LeafNode::getCapacity() const {

	return buffer_.getCapacity();
}


/*
 * ___________________________________________________________________________
 */
LeafNode::~LeafNode() {
}
