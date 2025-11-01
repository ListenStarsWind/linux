import lldb

debugger = lldb.debugger
target = debugger.GetSelectedTarget()
process = target.GetProcess()

print("--- 启动自动化线程调试脚本 ---\n")

for thread in process:
    thread_id = thread.GetThreadID()
    stop_reason = thread.GetStopReason()
    stack_top = thread.GetFrameAtIndex(0).GetSP()  # 改成 GetFrameAtIndex

    print(f"线程 [{thread_id}] 栈顶地址: {stack_top}")

    if stop_reason == lldb.eStopReasonSignal:
        signal = thread.GetStopReasonDataAtIndex(0)
        print(f"线程 [{thread_id}] 收到信号: {signal}\n")

    for idx in range(thread.GetNumFrames()):
        frame = thread.GetFrameAtIndex(idx)
        func_name = frame.GetFunctionName() or "未知函数"
        addr = frame.GetPC()
        print(f"  Frame {idx}: {func_name} at {hex(addr)}")

        # 局部变量
        vars_list = frame.GetVariables(True, True, True, True)
        if vars_list:
            for var in vars_list:
                print(f"    {var.GetName()} = {var.GetValue()}")
        else:
            print("    (无局部变量)")

    print("\n---\n")

print("--- 自动化线程调试脚本结束 ---")
