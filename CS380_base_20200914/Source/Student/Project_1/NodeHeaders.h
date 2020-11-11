#pragma once

// Include all node headers in this file

// Example Control Flow Nodes
#include "ControlFlow/C_ParallelSequencer.h"
#include "ControlFlow/C_RandomSelector.h"
#include "ControlFlow/C_Selector.h"
#include "ControlFlow/C_Sequencer.h"

// Student Control Flow Nodes
#include "ControlFlow/C_RandomSequencer.h"

// Example Decorator Nodes
#include "Decorator/D_Delay.h"
#include "Decorator/D_InvertedRepeater.h"
#include "Decorator/D_RepeatFourTimes.h"

// Student Decorator Nodes
#include "Decorator/D_UpdateYaw.h"
#include "Decorator/D_IfOutsideRange.h"
#include "Decorator/D_DelayRepeater.h"
#include "Decorator/D_SetupFish.h"
#include "Decorator/D_SetupShark.h"
#include "Decorator/D_SetupCrab.h"
#include "Decorator/D_SetupJelly.h"
#include "Decorator/D_IfNearbyTarget.h"
#include "Decorator/D_IfNearbyCrab.h"
#include "Decorator/D_RepeatUntilEdge.h"
#include "Decorator/D_NeighborCount.h"

// Example Leaf Nodes
#include "Leaf/L_CheckMouseClick.h"
#include "Leaf/L_Idle.h"
#include "Leaf/L_MoveToFurthestAgent.h"
#include "Leaf/L_MoveToMouseClick.h"
#include "Leaf/L_MoveToRandomPosition.h"

// Student Leaf Nodes
#include "Leaf/L_SeperateFromNearbyAgents.h"
#include "Leaf/L_MoveToCenter.h"
#include "Leaf/L_Move.h"
#include "Leaf/L_CalcSeperationVector.h"
#include "Leaf/L_CalcTetherVector.h"
#include "Leaf/L_CalcAlignmentVector.h"
#include "Leaf/L_CalcCohensionVector.h"
#include "Leaf/L_CalcWanderVector.h"
#include "Leaf/L_UpdateStatus.h"
#include "Leaf/L_Hunt.h"
#include "Leaf/L_RunAway.h"
#include "Leaf/L_ChangeColor.h"
#include "Leaf/L_SetSideways.h"