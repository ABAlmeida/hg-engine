#include "config.h"

#ifdef FIX_FIELD_MODEL_VBLANK_QUEUE_OVERFLOW

#include "io_reg.h"
#include "system.h"
#include "task.h"
#include "types.h"

enum {
    FIELD_MODEL_UPLOAD_PRIORITY = 0xFFFF,
    FIELD_VBLANK_RESERVED_TASKS = 4,
    OS_IRQ_VBLANK = 1 << REG_OS_IE_VB_SHIFT,
};

extern void LONG_CALL FieldModel_UploadOnVBlank(SysTask *task, void *resource);

/**
 * @brief Schedule one field-model upload without starving field lifecycle work.
 *
 * The original loader may submit 32 model uploads before VBlank drains its
 * 32-entry queue. Waiting at the high-water mark preserves four entries for
 * the field's own coordination tasks. No resource is deferred beyond the next
 * VBlank and ownership remains with the original model-resource manager.
 */
SysTask *LONG_CALL FieldModel_ScheduleVBlankUpload(void *resource)
{
    SysTaskQueue *queue = gSystem.vblankTaskQueue;
    SysTask *task;

    for (;;) {
        while (queue->limit <= FIELD_VBLANK_RESERVED_TASKS
            || queue->activeCount >= queue->limit - FIELD_VBLANK_RESERVED_TASKS) {
            OS_WaitIrq(TRUE, OS_IRQ_VBLANK);
        }

        task = SysTask_CreateOnVBlankQueue(
            FieldModel_UploadOnVBlank, resource, FIELD_MODEL_UPLOAD_PRIORITY);
        if (task != NULL) {
            return task;
        }

        // A concurrent VBlank can only free slots. If insertion still fails,
        // wait and retry rather than letting the caller continue without the
        // required texture upload.
        OS_WaitIrq(TRUE, OS_IRQ_VBLANK);
    }
}

#endif
