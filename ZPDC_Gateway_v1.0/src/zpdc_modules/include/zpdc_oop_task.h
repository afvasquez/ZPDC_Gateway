/*
 * zpdc_oop_task.h
 *
 * Created: 5/18/2017 1:52:54 PM
 *  Author: avasquez
 */ 


#ifndef ZPDC_OOP_TASK_H_
#define ZPDC_OOP_TASK_H_
#ifdef __cplusplus

class Task {
	public:
	BaseType_t t_init(const char* name, uint8_t priority) {
		return xTaskCreate(
		&taskfun,
		name,
		configMINIMAL_STACK_SIZE,
		this,
		tskIDLE_PRIORITY + priority,
		&handle);
	}

	virtual void task(void) =0;
	static void taskfun(void *parm) {
		((Task*)parm)->task();
		vTaskDelete(NULL);
	}

	TaskHandle_t handle;
};

#endif // __cplusplus
#endif /* ZPDC_OOP_TASK_H_ */