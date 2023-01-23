#include "CResourceBuffer.h"

CBuffer CBufferManager::CreateConstantBuffer()
{
    std::lock_guard<std::mutex> lockGuard(Mutex);

    return CBuffer();
}

CBufferManager::CBufferManager()
{
}
