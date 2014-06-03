
//===----------------------------------------------------------------------===//
// Register Classes
//===----------------------------------------------------------------------===//

//32-bit integer
def GPROnlyAcc : RegisterClass<"Qpu", [i32], 32, (add ACC0, ACC1, ACC2, ACC3)>;
def GPROnlyRA : RegisterClass<"Qpu", [i32], 32, (add RA0, RA1, RA2, RA3)>;
def GPROnlyRB : RegisterClass<"Qpu", [i32], 32, (add RB0, RB1, RB2, RB3)>;

def GPRAccRARB : RegisterClass<"Qpu", [i32], 32, (add ACC0, ACC1, ACC2, ACC3,
		RA0, RA1, RA2, RA3,
		RB0, RB1, RB2, RB3)>;

def GPRAccRA : RegisterClass<"Qpu", [i32], 32, (add ACC0, ACC1, ACC2, ACC3,
		RA0, RA1, RA2, RA3)>;

def GPRAccRB : RegisterClass<"Qpu", [i32], 32, (add ACC0, ACC1, ACC2, ACC3,
		RB0, RB1, RB2, RB3)>;

def GPRAcc5 : RegisterClass<"Qpu", [i32], 32, (add ACC5)>;


//32-bit float
def GPROnlyAcc_FP : RegisterClass<"Qpu", [f32], 32, (add ACC0, ACC1, ACC2, ACC3)>;
def GPROnlyRA_FP : RegisterClass<"Qpu", [f32], 32, (add RA0, RA1, RA2, RA3)>;
def GPROnlyRB_FP : RegisterClass<"Qpu", [f32], 32, (add RB0, RB1, RB2, RB3)>;

def GPRAccRARB_FP : RegisterClass<"Qpu", [f32], 32, (add ACC0, ACC1, ACC2, ACC3,
		RA0, RA1, RA2, RA3,
		RB0, RB1, RB2, RB3)>;

def GPRAccRA_FP : RegisterClass<"Qpu", [f32], 32, (add ACC0, ACC1, ACC2, ACC3,
		RA0, RA1, RA2, RA3)>;

def GPRAccRB_FP : RegisterClass<"Qpu", [f32], 32, (add ACC0, ACC1, ACC2, ACC3,
		RB0, RB1, RB2, RB3)>;

def GPRAcc5_FP : RegisterClass<"Qpu", [f32], 32, (add ACC5)>;


