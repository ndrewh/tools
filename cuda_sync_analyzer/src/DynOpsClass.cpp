#include "DynOpsClass.h"
DynOpsClass::DynOpsClass() {
	init = false;
}
std::vector<BPatch_function*> DynOpsClass::GetFunctionsByOffeset(BPatch_addressSpace * aspace, BPatch_object * obj, uint64_t offset) {
	std::vector<BPatch_function*> ret;
	BPatch_image * _img = aspace->getImage();
	uint64_t offsetAddress = obj->fileOffsetToAddr(offset);
	BPatch_function * func = _img->findFunction(offsetAddress);
	if (func == NULL){
		std::cerr << "Could not find function at supplied offset - " << std::dec << offset << " (symtab offset of: " << std::hex << offsetAddress << ")" << std::dec << std::endl;
		std::vector<BPatch_module *> tmpMod;
		obj->modules(tmpMod);
		for (auto i : tmpMod) {
			BPatch_Vector<BPatch_function*> fvec;
			i->getProcedures(fvec);
			for (auto n : fvec) {
				std::cerr << "[DEBUG] Function starting at " << std::dec << ((uint64_t)n->getBaseAddr()) - ((uint64_t) n->getModule()->getBaseAddr()) << " loaded at " << std::hex << n->getBaseAddr() << std::endl;
			}
			exit(0);
		}
		return ret;
	}
	ret.push_back(func);
	return ret;
}

bool DynOpsClass::IsNeverInstriment(BPatch_function * func) {
	static std::vector<std::string> librariesToSkip = {"libstdc++","libgcc","xerces-c","libelf", "libscalapack", "dyninst_10", "dyninst/build", "libessl", "dyninst/install","cudadedup-develop", "/opt/mellanox", "/usr/lib64/librt-", "spectrum-mpi", "ld-2.17", "libpthread-2.17.so"};
    std::string tmpLibname = func->getModule()->getObject()->pathName();
    for (auto i : librariesToSkip)
    	if (tmpLibname.find(i) != std::string::npos)
    		return true;
    return false;
	// std::string tmpFuncName = func->getName();


}


void DynOpsClass::GenerateAddrList(BPatch_addressSpace * aspace) {
	BPatch_Vector<BPatch_function*> fvec;
	BPatch_image * img = aspace->getImage();
	img->getProcedures(fvec);
	std::set<BPatch_basicBlock *>  blocks;
	for (auto i : fvec) {
		blocks.clear();
		GetBasicBlocks(i, blocks);
		for (auto n : blocks) {
			auto start = n->getStartAddress();
			auto end =  n->getEndAddress(); 
			for (uint64_t z = start; z <= end; z++){
				//if (_addressList.find(z) != _addressList.end())
				//	std::cerr << "Conflix between functions - " << _addressList[z]->getName() << " and " << i->getName() << " at addr " << z << std::endl;
				_addressList[z] = i;
			}
		}
	}
}

BPatchPointVecPtr DynOpsClass::GetPoints(BPatch_function * func, const BPatch_procedureLocation pos) {
	BPatchPointVecPtr ret;
	ret.reset(func->findPoint(pos));
	return ret;
}
void DynOpsClass::PowerFunctionCheck(BPatch_addressSpace * addr, BPatch_function * & funcToCheck) {
	BPatch_image * _img = addr->getImage();
	std::vector<BPatch_function *> ret;
	uint64_t baseAddr = (uint64_t)funcToCheck->getBaseAddr();
	_img->findFunction(baseAddr + 0x8, ret);
	assert(ret.size() > 0);
	if (ret.size() == 1)
		return;
	// there should never be 3 functions at +0x8. If there is, something will screw up with instrimentation
	assert(ret.size() == 2);

	if ((uint64_t)ret[0]->getBaseAddr() == baseAddr + 0x8)
		funcToCheck = ret[0];
	else if ((uint64_t)ret[1]->getBaseAddr() == baseAddr + 0x8)
		funcToCheck = ret[1];

}

std::map<uint64_t, std::string> DynOpsClass::GetRealAddressAndLibName(BPatch_addressSpace * aspace) {
	std::map<uint64_t, std::string> ret;
	BPatch_image * img = aspace->getImage();
	std::vector<BPatch_module*> modules;
	img->getModules(modules);
	for (auto i : modules){
		if (i->isSharedLib())
			ret[(uint64_t)(i->getBaseAddr())] = i->getObject()->pathName();
		else 
			ret[0] = i->getObject()->pathName();
	}
	return ret;
}

std::vector<BPatch_function *> DynOpsClass::FindFuncsInObjectByName(BPatch_addressSpace * aspace, BPatch_object * obj, std::string name) {
	std::vector<BPatch_function *> ret;
	obj->findFunction(name, ret);
	// for (auto & i : ret)
	//  	PowerFunctionCheck(aspace, i);
	return ret;
}

std::vector<BPatch_function *> DynOpsClass::FindFuncsByName(BPatch_addressSpace * aspace, std::string name, BPatch_object * obj) {
	if (obj != NULL)
		return FindFuncsInObjectByName(aspace, obj, name);
	BPatch_image * img = aspace->getImage();
	std::vector<BPatch_function *> ret;
	img->findFunction(name.c_str(), ret);
	// for (auto & i : ret)
	//  	PowerFunctionCheck(aspace, i);
	return ret;
}

void DynOpsClass::SetupPowerMap(BPatch_addressSpace * addr) {
	if (init)
		return;

	std::vector<BPatch_function *> all_functions;
	BPatch_image * _img = addr->getImage();
	_img->getProcedures(all_functions);
	for (auto i : all_functions) {
		if (_powerFuncmap.find((uint64_t)i->getBaseAddr()) != _powerFuncmap.end())
			assert("WE SHOULDN'T BE HERE" != 0);
		_powerFuncmap[(uint64_t)i->getBaseAddr()] = i;
	}
	init = true;
}

BPatch_function * DynOpsClass::GetPOWERFunction(BPatch_function * function) {
	SetupPowerMap(function->getAddSpace());
	uint64_t address = (uint64_t)function->getBaseAddr();
	if (_powerFuncmap.find(address + 0x8) != _powerFuncmap.end())
		return _powerFuncmap[address + 0x8];
	return function;

}
// BPatch_function * DynOpsClass::GetPOWERFunction(BPatch_function * function) { 
// 	// Returns the real function when using power.



// }

int DynOpsClass::FindFuncByName(BPatch_addressSpace * aspace, BPatch_function * & ret, std::string name) {
	if (aspace == NULL) 
		return -1;
	SetupPowerMap(aspace);
	BPatch_image * img = aspace->getImage();
	std::vector<BPatch_function *> tmp;
	img->findFunction(name.c_str(), tmp);
	if (tmp.size() > 0) 
		ret = GetPOWERFunction(tmp[0]);
	return tmp.size();
}

BPatch_object * DynOpsClass::FindObjectByName(BPatch_addressSpace * aspace, std::string & name, bool exact) {
	if (aspace == NULL) 
		return NULL;
	BPatch_image * img = aspace->getImage();
	std::vector<BPatch_object *> objects;
	img->getObjects(objects);
	for (auto i : objects){ 
		if (exact) {
			if (i->pathName() == name)
				return i;
		} else {
			if (i->pathName().find(name) != std::string::npos)
				return i;
		}
	}
	return NULL;
}

int DynOpsClass::FindFuncByLibnameOffset(BPatch_addressSpace * aspace, BPatch_function * & ret, std::string libname, uint64_t offset, bool exact) {
	if (aspace == NULL) 
		return -1;
	BPatch_image * img = aspace->getImage();
	BPatch_object * obj = FindObjectByName(aspace, libname, exact);
	
	if (obj == NULL)
		return -1;

	BPatch_function * func = img->findFunction(obj->fileOffsetToAddr(offset));
	if (func == NULL)
		return 0;
	ret = func;

	return 1;
}


bool CheckIfSharedLib(BPatch_object * obj) {
	std::vector<BPatch_module *> mods;
	obj->modules(mods);
	for (auto i : mods) {
		if (i->isSharedLib())
			return true;
	}
	return false;
}


std::vector<BPatch_function *> DynOpsClass::FindFunctionsByLibnameOffset(BPatch_addressSpace * aspace, std::string libname, uint64_t offset, bool exact) {
	std::vector<BPatch_function *> ret;
	if (aspace == NULL) 
		return ret;

	std::cerr << "Trying to find - " << libname << " at offset " << std::hex << offset << std::endl;
	BPatch_image * img = aspace->getImage();
	BPatch_object * obj = FindObjectByName(aspace, libname, exact);
	
	if (obj == NULL){
		std::cerr << "Could not find object - " << libname << std::endl;
		return ret;
	}
	std::vector<BPatch_point *> points;
	if (CheckIfSharedLib(obj))
		obj->findPoints(obj->fileOffsetToAddr(offset), points);
	else
		obj->findPoints(offset, points);
	//img->findFunction(obj->fileOffsetToAddr(offset), ret);
	for (auto i : points)
		ret.push_back(i->getFunction());
	std::cerr << "Found - " << ret.size() << " functions" << std::endl;
	return ret;
}

void DynOpsClass::GetBasicBlocks(BPatch_function * func, std::set<BPatch_basicBlock *> & ret) {
	assert(func->getCFG() != NULL);
	func->getCFG()->getAllBasicBlocks(ret);
}


// Hack to get around point->getInsnAtPoint() not wokring
Dyninst::InstructionAPI::Instruction DynOpsClass::FindInstructionAtPoint(BPatch_point * point) {
	std::set<BPatch_basicBlock *> blocks;
	std::vector<std::pair<Dyninst::InstructionAPI::Instruction, Dyninst::Address> > instructionVector;
	GetBasicBlocks(point->getFunction(), blocks);

	for (auto i : blocks) {
		instructionVector.clear();
		i->getInstructions(instructionVector);
		for (auto n : instructionVector) {
			if (n.second == (uint64_t)point->getAddress())
				return n.first;
		}
	}
	// //point->getBlock()->getInstructions(instructionVector);
	// bool found = false;
	// for (auto z : instructionVector) {
	// 	if(z.second == (uint64_t)point->getAddress()) {
	// 		found = true;
	// 		return z.first;
	// 	} else if (found == true) {
			 
	// 	}
	// }
	return Dyninst::InstructionAPI::Instruction();
}

std::vector<BPatch_object *> DynOpsClass::GetObjects(BPatch_addressSpace * aspace) {
	BPatch_image * img = aspace->getImage();
	std::vector<BPatch_object *> objects;
	img->getObjects(objects);
	return objects;
}


bool DynOpsClass::GetFileOffset(BPatch_addressSpace * aspace, BPatch_point * point, uint64_t & addr, bool addInstSize) {
	if (point->getFunction() == NULL)
		return false;
	//auto inst = FindInstructionAtPoint(point);
	size_t size = 0;
	if (addInstSize){
		size = 4;
	}
	if (point->getFunction()->getModule()->isSharedLib())
		addr = (uint64_t)point->getAddress() - (uint64_t)point->getFunction()->getModule()->getBaseAddr() + size;
	else
		addr = (uint64_t)point->getAddress() + size;
	return true;
}

std::unordered_map<uint64_t, BPatch_function *> DynOpsClass::GetFuncMap(
        BPatch_addressSpace * aspace, BPatch_object * object) {

    std::unordered_map<uint64_t, BPatch_function *> funcMap;
    //BPatch_object * libCuda = LoadLibrary(std::string("libcuda.so.1"));
    std::vector<BPatch_function *> funcs;
    aspace->getImage()->getProcedures(funcs);

    if (object == NULL) {
        for (auto i : funcs)
            funcMap[(uint64_t)i->getBaseAddr()] = i;
    }
    else {
        for (auto i : funcs) {
            if (i->getModule()->getObject() == object)
                funcMap[((uint64_t)i->getBaseAddr()) - ((uint64_t)i->getModule()->getBaseAddr())] = i;
        }
    }
    return funcMap;
}

BPatch_point * DynOpsClass::FindPreviousPoint(
        BPatch_point* point, BPatch_addressSpace * aspace) {
    auto image = aspace->getImage();
    std::vector<BPatch_point *> points;
    image->findPoints((((uint64_t)point->getAddress()) - 0x4), points);
    if (points.size() > 0)
        return points[0];
    return point;
    //assert("SHOULD FIND A POINT BUT ARE NOT!!!" == 0);
}

