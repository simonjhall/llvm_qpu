
//===----------------------------------------------------------------------===//
// Register Classes
//===----------------------------------------------------------------------===//

//32-bit integer
def GPROnlyAcc : RegisterClass<"Qpu", [i32], 32, (add ACC0, ACC1, ACC2, ACC3)>;
def GPROnlyRA : RegisterClass<"Qpu", [i32], 32, (add RA0, RA1, RA2, RA3, RA4, RA5, RA6, RA7)>;
def GPROnlyRB : RegisterClass<"Qpu", [i32], 32, (add RB0, RB1, RB2, RB3, RB4, RB5, RB6, RB7)>;

def GPRAccRARB : RegisterClass<"Qpu", [i32], 32, (add ACC0, ACC1, ACC2, ACC3,
		RA0, RA1, RA2, RA3, RA4, RA5, RA6, RA7,
		RB0, RB1, RB2, RB3, RB4, RB5, RB6, RB7)>;

def GPRAccRA : RegisterClass<"Qpu", [i32], 32, (add ACC0, ACC1, ACC2, ACC3,
		RA0, RA1, RA2, RA3, RA4, RA5, RA6, RA7)>;

def GPRAccRB : RegisterClass<"Qpu", [i32], 32, (add ACC0, ACC1, ACC2, ACC3,
		RB0, RB1, RB2, RB3, RB4, RB5, RB6, RB7)>;

def GPRAcc5 : RegisterClass<"Qpu", [i32], 32, (add ACC5)>;


multiclass vec_sub_regfile<ValueType type, int alignment>
{
	def _GPROnlyAcc_FP : RegisterClass<"Qpu", [type], alignment, (add ACC0, ACC1, ACC2, ACC3)>;
	def _GPROnlyRA_FP : RegisterClass<"Qpu", [type], alignment, (add RA0, RA1, RA2, RA3, RA4, RA5, RA6, RA7)>;
	def _GPROnlyRB_FP : RegisterClass<"Qpu", [type], alignment, (add RB0, RB1, RB2, RB3, RB4, RB5, RB6, RB7)>;
	
	def _GPRAccRARB_FP : RegisterClass<"Qpu", [type], alignment, (add ACC0, ACC1, ACC2, ACC3,
			RA0, RA1, RA2, RA3, RA4, RA5, RA6, RA7,
			RB0, RB1, RB2, RB3, RB4, RB5, RB6, RB7)>;
	
	def _GPRAccRA_FP : RegisterClass<"Qpu", [type], alignment, (add ACC0, ACC1, ACC2, ACC3,
			RA0, RA1, RA2, RA3, RA4, RA5, RA6, RA7)>;
	
	def _GPRAccRB_FP : RegisterClass<"Qpu", [type], alignment, (add ACC0, ACC1, ACC2, ACC3,
			RB0, RB1, RB2, RB3, RB4, RB5, RB6, RB7)>;
	
	def _GPRAcc5_FP : RegisterClass<"Qpu", [type], alignment, (add ACC5)>;
}

defm F32x1 : vec_sub_regfile<f32, 32>;
defm F32x2 : vec_sub_regfile<v2f32, 512>;
defm F32x4 : vec_sub_regfile<v4f32, 512>;
defm F32x8 : vec_sub_regfile<v8f32, 512>;
defm F32x16 : vec_sub_regfile<v16f32, 512>;

