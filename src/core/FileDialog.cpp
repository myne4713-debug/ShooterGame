#include "core/FileDialog.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <commdlg.h>
#include <system_error>

std::optional<std::filesystem::path> openImageFileDialog(const std::filesystem::path& initialDir) {
    wchar_t filePath[MAX_PATH * 4]{};
    static constexpr wchar_t kFilter[] =
        L"Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0"
        L"All Files (*.*)\0*.*\0";

    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = kFilter;
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = static_cast<DWORD>(sizeof(filePath) / sizeof(filePath[0]));
    ofn.lpstrTitle = L"Chon anh de tao map 3D";
    // Giữ nguyên thư mục làm việc hiện tại để các đường dẫn asset tương đối không bị lệch.
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR | OFN_EXPLORER;

    std::wstring initDirStr;
    std::error_code ec;
    if (!initialDir.empty() && std::filesystem::exists(initialDir, ec)) {
        initDirStr = std::filesystem::weakly_canonical(initialDir, ec).wstring();
        if (!initDirStr.empty()) {
            ofn.lpstrInitialDir = initDirStr.c_str();
        }
    }

    if (GetOpenFileNameW(&ofn)) {
        return std::filesystem::path(filePath);
    }
    return std::nullopt;
}
#else
std::optional<std::filesystem::path> openImageFileDialog(const std::filesystem::path&) {
    return std::nullopt;
}
#endif
