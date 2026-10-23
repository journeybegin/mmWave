/*
 *  ======== main.c ========
 */
#include <string.h>


#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>


#include <ti/drivers/soc/soc.h>
/*
 *  ======== taskFxn ========
 */
void dss_initTask(UArg a0, UArg a1)
{
    System_printf("enter taskFxn()\n");

    Task_sleep(10);

    System_printf("exit taskFxn()\n");

    System_flush(); /* force SysMin output to console */
}

/*
 *  ======== main ========
 */
int main(void)
{ 
    Task_Params taskParams;
    int32_t     errCode;
    SOC_Cfg     socCfg;
    SOC_Handle  socHandle;

    memset ((void *)&socCfg, 0, sizeof(SOC_Cfg));

    socCfg.clockCfg = SOC_SysClock_INIT;
    socHandle = SOC_init (&socCfg, &errCode);
    if (socHandle == NULL)
    {
        System_printf ("Error: SOC Module Initialization failed [Error code %d]\n", errCode);
        return 0;
    }

    /* Initialize the Task Parameters. */
    Task_Params_init(&taskParams);
    Task_create(dss_initTask, &taskParams, NULL);

    /* Start BIOS */
    BIOS_start();
    return 0;
}
