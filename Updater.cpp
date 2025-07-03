#include <iostream>
#include <string>
#include <git2.h>
#include <windows.h>
#include <pathcch.h>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

void handleGitError(int error);
void waitForProcessToExit(DWORD pid);

int credentialsCallback(git_cred** out,
    const char* url,
    const char* username_from_url,
    unsigned int allowed_types,
    void* payload)
{
    return git_credential_ssh_key_memory_new(
        out,
        "", // TODO: Username
        "", // TODO: Public key
        "", // TODO: Private key
        "" // TODO: Passphrase
    );
}

int main(int argc, char* argv[])
{
    char* installPath = argv[1];
    DWORD pid = atoi(argv[2]);
    cout << "Waiting for program (" << pid << ") to have exited..." << endl;
    waitForProcessToExit(pid);

    cout << "Updating files..." << endl;

    int error;

    git_libgit2_init();
    error = git_libgit2_opts(GIT_OPT_SET_OWNER_VALIDATION, 0);
    handleGitError(error);

    git_repository* repo = NULL;
    error = git_repository_open(&repo, installPath);
    handleGitError(error);

    git_remote* remote;
    error = git_remote_lookup(&remote, repo, "origin");
    handleGitError(error);
    git_remote_callbacks callbacks = GIT_REMOTE_CALLBACKS_INIT;
    git_fetch_options fetch_opts = GIT_FETCH_OPTIONS_INIT;
    // fetch_opts.callbacks.progress = progressCallback;
    fetch_opts.callbacks.credentials = credentialsCallback;
    error = git_remote_fetch(remote,
        NULL,
        &fetch_opts,
        NULL
    );
    handleGitError(error);

    git_reference* headRef = NULL;
    error = git_reference_dwim(&headRef, repo, "HEAD");
    handleGitError(error);
    git_reference* newRef = NULL;
    git_reference* ref = NULL;
    error = git_reference_lookup(&ref, repo, "refs/remotes/origin/main");
    handleGitError(error);
    error = git_reference_set_target(
        &newRef,
        headRef,
        git_reference_target(ref),
        "Updating HEAD to match origin/main"
    );
    handleGitError(error);

    git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
    opts.checkout_strategy = GIT_CHECKOUT_FORCE;
    error = git_checkout_head(repo, &opts);
    handleGitError(error);

    git_libgit2_shutdown();

    char* startPath = argv[3];
    
    string commandLine2 = "";
    for (int index = 4; index < argc; index++) {
        if (index > 4) {
            commandLine2 += " ";
        }
        commandLine2 += "\"" + string(argv[index]) + "\"";
    }
    char* commandLine = _strdup(commandLine2.c_str());

    STARTUPINFOA startupInfo;
    ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
    startupInfo.cb = sizeof(STARTUPINFO);

    PROCESS_INFORMATION processInfo;
    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));

    cout << "Starting program..." << endl;

    bool hasProcessBeenCreated = CreateProcessA(
        startPath,
        commandLine,
        NULL,
        NULL,
        false,
        NORMAL_PRIORITY_CLASS | CREATE_NEW_PROCESS_GROUP,
        NULL,
        NULL,
        &startupInfo,
        &processInfo
    );

    if (!hasProcessBeenCreated) {
        printf("Failed to start the program after update: %d\n", GetLastError());
        cout << "Please start the program manually again." << endl;
        return 1;
    }

    return 0;
}

void handleGitError(int error) {
    if (error < 0) {
        const git_error* e = git_error_last();
        printf("Error %d/%d: %s\n", error, e->klass, e->message);
        exit(error);
    }
}

void waitForProcessToExit(DWORD pid) {
    HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, pid);
    if (hProcess == NULL) {
        return;
    }
    WaitForSingleObject(hProcess, INFINITE);
    CloseHandle(hProcess);
}
