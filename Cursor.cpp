#include <stdexcept>
#include <iostream>
#include <vector>
#include "Cursor.h"
#include "DataUnit.h"
#include "DataUnitFilter.h"
#include "String_.h"

using std::cout;
using std::endl;
using std::vector;


/*
 * ___________________________________________________________________________
 */
Cursor::Cursor(DataUnit& root)
		: root_(root), position_(0), globalFilter_(0) {

	/* move to first position after root matching criteria */
	this->seekByIndex((size_t)0);
}


/*
 * ___________________________________________________________________________
 */
Cursor::Cursor(DataUnit& root, DataUnit& position)
		: root_(root), position_(0), globalFilter_(0) {

	this->moveTo(position);
}


/*
 * ___________________________________________________________________________
 */
Cursor::Cursor(DataUnit& root, const DataUnitFilter& filter)
		: root_(root), position_(0), globalFilter_(&filter) {

	/* move to first position after root matching criteria */
	this->seekByIndex((size_t)0);
}


/*
 * ___________________________________________________________________________
 */
Cursor::Cursor(DataUnit& root, DataUnit& position,
		const DataUnitFilter& filter)
		: root_(root), position_(0), globalFilter_(&filter) {

	this->moveTo(position);
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::applyFilter(const DataUnit* dataUnit,
		const DataUnitFilter* filter) const {

	bool passes = true;

	if (dataUnit != 0) {
		if (globalFilter_ != 0 && !globalFilter_->apply(*dataUnit)) {
			passes = false;
		}
		if (filter != 0 && !filter->apply(*dataUnit)) {
			passes = false;
		}
		if (!dataUnit->isDescendantOf(root_) && dataUnit != &root_) {
			passes = false;
		}
	}

	return passes;
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::moveTo(DataUnit* position) {

	bool accepted = false;
	if (position != 0 && applyFilter(position)) {
		position_ = position;
		accepted = true;
	}
	return accepted;
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::setTo(DataUnit* position) {

	bool accepted = this->moveTo(position);
	if (!accepted) {
		position_ = (DataUnit*)0;
	}
	return accepted;
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::seekByIndex_(size_t n, const DataUnitFilter* filter) {

	DataUnit* pos = &root_;
	/* move to index 0, i.e. first position from root matching criteria */
	while (!applyFilter(pos, filter)) {
		pos = pos->getSuccessor();
	}
	/* now move ahead n positions matching criteria */
	while ((pos != 0) && (n > 0)) {
		pos = pos->getSuccessor();
		if (applyFilter(pos, filter)) {
			--n;
		}
	}
	return this->moveTo(pos);
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::moveToEnd() {

	DataUnit* pos = position_;
	DataUnit* tst = position_;
	while (tst != 0) {
		pos = tst;
		tst = tst->getSuccessor();
	}
	return this->moveTo(pos);
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::moveRight(size_t steps, const DataUnitFilter* filter) {

	DataUnit* pos = position_;
	while ((pos != 0) && (steps > 0)) {
		pos = pos->getNext();
		if (applyFilter(pos, filter)) {
			--steps;
		}
	}
	return this->moveTo(pos);
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::moveLeft(size_t steps, const DataUnitFilter* filter) {

	DataUnit* pos = position_;
	while ((pos != 0) && (steps > 0)) {
		pos = pos->getPrevious();
		if (applyFilter(pos, filter)) {
			--steps;
		}
	}
	return this->moveTo(pos);
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::moveUp(size_t steps, const DataUnitFilter* filter) {

	DataUnit* pos = position_;
	while ((pos != 0) && (steps > 0)) {
		pos = pos->getParent();
		if (applyFilter(pos, filter)) {
			--steps;
		}
	}
	return this->moveTo(pos);
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::moveDown(size_t steps, const DataUnitFilter* filter) {

	DataUnit* pos = position_;
	while ((pos != 0) && (steps > 0)) {
		pos = pos->getChild();
		if (applyFilter(pos, filter)) {
			--steps;
		}
	}
	return this->moveTo(pos);
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::moveForward(size_t steps, const DataUnitFilter* filter) {

	DataUnit* pos = position_;
	while ((pos != 0) && (steps > 0)) {
		pos = pos->getSuccessor();
		if (applyFilter(pos, filter)) {
			--steps;
		}
	}
	return this->moveTo(pos);
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::moveBackward(size_t steps, const DataUnitFilter* filter) {

	DataUnit* pos = position_;
	while ((pos != 0) && (steps > 0)) {
		pos = pos->getPredecessor();
		if (applyFilter(pos, filter)) {
			--steps;
		}
	}
	return this->moveTo(pos);
}


/*
 * ___________________________________________________________________________
 */
size_t Cursor::scan(std::vector<DataUnit*>* dataUnits,
		const DataUnitFilter* filter) const {

	size_t n = 0;

	DataUnit* pos = &root_;
	while (pos != 0) {
		if (applyFilter(pos, filter)) {
			if (dataUnits != 0) {
				dataUnits->push_back(pos);
			}
			++n;
		}
		pos = pos->getSuccessor();
	}

	return n;
}


/*
 * ___________________________________________________________________________
 */
size_t Cursor::printScan_(
		const DataUnitFilter* filter, bool printAll) const {

	vector<DataUnit*> dataUnits;
	this->scan(&dataUnits);

	size_t max = 9;
	size_t w = 1;
	while (dataUnits.size() > max) {
		++w;
		max = (max + 1) * 10 - 1;
	}

	for (size_t i = 0; i < dataUnits.size(); ++i) {

		bool passFilter = true;
		if (filter != 0 && !filter->apply(*(dataUnits[i]))) {
			passFilter = false;
		}

		/* print cursor position */
		if (printAll || passFilter) {

			/* print index */
			cout << String::format("%*u: ", w, i);

			if (position_ != 0) {
				if (dataUnits[i] == position_) {
					cout << "=> ";
				} else {
					cout << "   ";
				}
			}
		}

		/* print paths (including filter results) */
		if (printAll && passFilter) {
			cout << String::makeBoldRed(dataUnits[i]->getPath()) << endl;
		} else if (printAll || passFilter) {
			cout << dataUnits[i]->getPath() << endl;
		}
	}

	return dataUnits.size();
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::doInsert(DataUnit* insertion) {

	bool success = false;

	/* cannot insert at root. TODO: make this possible */
	if (this->valid() && position_ != &root_) {
		DataUnit* current = position_;
		if (current->insert(insertion)) {
			success = true;
			/* set to insertion */
			this->setTo(insertion);
		}
	}

	return success;
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::doReplace(DataUnit* replacement) {

	bool success = false;

	/* cannot replace root. TODO: make this possible */
	if (this->valid() && position_ != &root_) {
		DataUnit* current = position_;
		if (current->replaceBy(replacement)) {
			success = true;
			/* delete previously current data unit */
			delete current;
			/* set to replacement */
			this->setTo(replacement);
		}
	}

	return success;
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::doTruncate(const BC& length) {

	bool success = false;

	if (this->valid()) {
		position_->truncate(length);
		success = true;
	}

	return success;
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::doRemove() {

	bool success = false;

	/* cannot remove root TODO: make this possible */
	if (this->valid() && position_ != &root_) {
		DataUnit* current = position_;
		DataUnit* parent = position_->getParent();
		/* remove node at current position */
		current->remove();
		success = true;
		/* set to parent of removed node */
		this->setTo(parent);
	}

	return success;
}


/*
 * ___________________________________________________________________________
 */
bool Cursor::doDelete() {

	bool success = false;

	DataUnit* current = position_;
	if (this->doRemove()) {
		delete current;
		success = true;
	}

	return success;
}


/*
 * ___________________________________________________________________________
 */
Cursor::~Cursor() {
}

