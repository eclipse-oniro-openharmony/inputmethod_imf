/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <asm/unistd.h>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <sched.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <syscall.h>
#include <unistd.h>

#include <cerrno>
#include <climits>
#include <csignal>
#include <cstdlib>
#include <cstring>

#include "seccomp_policy.h"

using SyscallFunc = bool (*)(void);
constexpr int SLEEP_TIME_100MS = 100000; // 100ms
constexpr int SLEEP_TIME_1S = 1;

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace MiscServices {
class SeccompUnitTest : public testing::Test {
public:
    SeccompUnitTest(){};
    virtual ~SeccompUnitTest(){};
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};

    void SetUp()
    {
        /*
         * Wait for 1 second to prevent the generated crash file
         * from being overwritten because the crash interval is too short
         * and the crash file's name is constructed by time stamp.
         */
        sleep(SLEEP_TIME_1S);
    };

    void TearDown(){};
    void TestBody(void){};

    static pid_t StartChild(SeccompFilterType type, const char *filterName, SyscallFunc func)
    {
        pid_t pid = fork();
        if (pid == 0) {
            if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) != 0) {
                std::cout << "PR_SET_NO_NEW_PRIVS set fail " << std::endl;
                exit(EXIT_FAILURE);
            }

            if (!SetSeccompPolicyWithName(type, filterName)) {
                std::cout << "SetSeccompPolicy set fail fiterName is " << filterName << std::endl;
                exit(EXIT_FAILURE);
            }

            if (!func()) {
                std::cout << "func excute fail" << std::endl;
                exit(EXIT_FAILURE);
            }

            std::cout << "func excute success" << std::endl;

            exit(EXIT_SUCCESS);
        }
        return pid;
    }

    static int CheckStatus(int status, bool isAllow)
    {
        if (WEXITSTATUS(status) == EXIT_FAILURE) {
            return -1;
        }

        if (WIFSIGNALED(status)) {
            if (WTERMSIG(status) == SIGSYS) {
                std::cout << "child process exit with SIGSYS" << std::endl;
                return isAllow ? -1 : 0;
            }
        } else {
            std::cout << "child process finished normally" << std::endl;
            return isAllow ? 0 : -1;
        }

        return -1;
    }

    static int CheckSyscall(SeccompFilterType type, const char *filterName, SyscallFunc func, bool isAllow)
    {
        sigset_t set;
        int status;
        pid_t pid;
        int flag = 0;
        struct timespec waitTime = { 5, 0 };

        sigemptyset(&set);
        sigaddset(&set, SIGCHLD);
        sigprocmask(SIG_BLOCK, &set, nullptr);
        sigaddset(&set, SIGSYS);
        if (signal(SIGCHLD, SIG_DFL) == nullptr) {
            std::cout << "signal failed:" << strerror(errno) << std::endl;
        }
        if (signal(SIGSYS, SIG_DFL) == nullptr) {
            std::cout << "signal failed:" << strerror(errno) << std::endl;
        }

        /* Sleeping for avoiding influencing child proccess wait for other threads
         * which were created by other unittests to release global rwlock. The global
         * rwlock will be used by function dlopen in child process */
        usleep(SLEEP_TIME_100MS);

        pid = StartChild(type, filterName, func);
        if (pid == -1) {
            std::cout << "fork failed:" << strerror(errno) << std::endl;
            return -1;
        }
        if (sigtimedwait(&set, nullptr, &waitTime) == -1) { /* Wait for 5 seconds */
            if (errno == EAGAIN) {
                flag = 1;
            } else {
                std::cout << "sigtimedwait failed:" << strerror(errno) << std::endl;
            }
        }

        if (waitpid(pid, &status, 0) != pid) {
            std::cout << "waitpid failed:" << strerror(errno) << std::endl;
            return -1;
        }

        if (flag != 0) {
            std::cout << "Child process time out" << std::endl;
        }

        return CheckStatus(status, isAllow);
    }

    static bool CheckSendfile()
    {
        int ret = syscall(__NR_sendfile, 0, 0, nullptr, 0);
        if (ret == 0) {
            return true;
        }

        return false;
    }

    static bool CheckVmsplice()
    {
        int ret = syscall(__NR_vmsplice, 0, nullptr, 0, 0);
        if (ret == 0) {
            return true;
        }

        return false;
    }

    static bool CheckSocketpair()
    {
        int ret = syscall(__NR_socketpair, 0, 0, 0, nullptr);
        if (ret == 0) {
            return true;
        }

        return false;
    }

    static bool CheckListen()
    {
        int ret = syscall(__NR_listen, 0, 0);
        if (ret == 0) {
            return true;
        }

        return false;
    }

    static bool CheckAccept()
    {
        int ret = syscall(__NR_accept, 0, nullptr, nullptr);
        if (ret == 0) {
            return true;
        }

        return false;
    }

    static bool CheckAccept4()
    {
        int ret = syscall(__NR_accept4, 0, nullptr, nullptr, 0);
        if (ret == 0) {
            return true;
        }

        return false;
    }

    static bool CheckGetsockname()
    {
        int ret = syscall(__NR_getsockname, 0, nullptr, nullptr);
        if (ret == 0) {
            return true;
        }

        return false;
    }

    static bool CheckGetpeername()
    {
        int ret = syscall(__NR_getpeername, 0, nullptr, nullptr);
        if (ret == 0) {
            return true;
        }

        return false;
    }

    static bool CheckShutdown()
    {
        int ret = syscall(__NR_shutdown, 0, 0);
        if (ret == 0) {
            return true;
        }

        return false;
    }

    static bool CheckSendmsg()
    {
        int ret = syscall(__NR_sendmsg, 0, nullptr, 0);
        if (ret == 0) {
            return true;
        }

        return false;
    }

    static bool CheckRecvmmsg()
    {
        int ret = syscall(__NR_recvmmsg, 0, nullptr, 0, 0, nullptr);
        if (ret == 0) {
            return true;
        }

        return false;
    }
#if defined __aarch64__
    static bool CheckSetuid()
    {
        int uid = syscall(__NR_setuid, 1);
        if (uid == 0) {
            return true;
        }

        return false;
    }

#elif defined __arm__
    static bool CheckSetuid32()
    {
        uid_t uid = syscall(__NR_setuid32, 123);
        if (uid >= 0) {
            return true;
        }
        return false;
    }

    static bool CheckSendfile64()
    {
        int ret = syscall(__NR_sendfile64, 0, 0, nullptr, 0);
        if (ret == 0) {
            return true;
        }

        return false;
    }
    static bool CheckRecvmmsgTime64()
    {
        int ret = syscall(__NR_recvmmsg_time64, 0, nullptr, 0, 0, nullptr);
        if (ret == 0) {
            return true;
        }

        return false;
    }
#endif

    void TestInputMethodExtSycall()
    {
        int ret = -1;
        ret = CheckSyscall(APP, IMF_EXTENTOIN_NAME, CheckSendfile, false);
        EXPECT_EQ(ret, 0);

        ret = CheckSyscall(APP, IMF_EXTENTOIN_NAME, CheckVmsplice, false);
        EXPECT_EQ(ret, 0);

#if defined __aarch64__
        // system blocklist
        ret = CheckSyscall(APP, IMF_EXTENTOIN_NAME, CheckSetuid, false);
        EXPECT_EQ(ret, 0);
#elif defined __arm__
        // system blocklist
        ret = CheckSyscall(APP, IMF_EXTENTOIN_NAME, CheckSetuid32, false);
        EXPECT_EQ(ret, 0);
        ret = CheckSyscall(APP, IMF_EXTENTOIN_NAME, CheckSendfile64, false);
        EXPECT_EQ(ret, 0);
        ret = CheckSyscall(APP, IMF_EXTENTOIN_NAME, CheckRecvmmsgTime64, false);
        EXPECT_EQ(ret, 0);
#endif
    }
};

/**
 * @tc.name: TestInputMethodExtSeccomp
 * @tc.desc: Verify the input method extenstion's seccomp policy.
 * @tc.type: FUNC
 * @tc.require: issueI9PUAS
 */
HWTEST_F(SeccompUnitTest, TestInputMethodExtSycall, TestSize.Level1)
{
    SeccompUnitTest test;
    test.TestInputMethodExtSycall();
}
} // namespace MiscServices
} // namespace OHOS
