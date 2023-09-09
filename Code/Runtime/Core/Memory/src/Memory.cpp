#include "CookieKat/Core/Memory/Memory.h"

namespace CKE::Memory {
	void MemoryTrackingManager::RecordOperation(MemoryOperationInfo operationInfo) {
		m_OperationsHistory.push_back(operationInfo);

		if (operationInfo.m_Operation == MemoryOp::Alloc ||
			operationInfo.m_Operation == MemoryOp::New ||
			operationInfo.m_Operation == MemoryOp::NewArray) {
			m_TotalAllocated += operationInfo.m_Size;

			m_ActiveAllocations.insert({
				(MemoryAddress)operationInfo.m_pAddress,
				MemoryAllocationInfo{operationInfo.m_pTypeName, operationInfo.m_pAddress, operationInfo.m_Size, operationInfo.m_Alignment}
			});
		}
		else if (operationInfo.m_Operation == MemoryOp::Free ||
			operationInfo.m_Operation == MemoryOp::Delete ||
			operationInfo.m_Operation == MemoryOp::DeleteArray) {
			m_TotalReleased += operationInfo.m_Size;
			m_ActiveAllocations.erase((MemoryAddress)operationInfo.m_pAddress);
		}
	}
}
