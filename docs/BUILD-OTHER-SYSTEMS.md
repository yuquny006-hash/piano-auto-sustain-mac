# 其他系统构建（1.0）

GitHub 私有仓库：yuquny006-hash/amu-music-bridge
独立分支：piano-auto-sustain-1.0-build；原 amu-piano 分支未修改。
工作流：Piano Auto Sustain 1.0 cross-platform。
源代码目录：piano-auto-sustain。工作流副本：scripts/github-build-workflow.yml。

首轮 Mac Universal VST3 / AU 和 Linux x64 VST3 编译完成；处理器测试通过，VST3 扫描因传入内部二进制路径而失败。已改为 bundle 目录。
2026-10-12 修正后的运行 38202304297 未启动：GitHub 提示 recent account payments have failed or spending limit needs to be increased。账单页面显示实际计费为 0；具体限制尚未查清。用户决定暂缓 Mac 端，暂不重跑付费云编译。
尚无可下载 Mac/Linux 安装包；签名验证、AU auval、打包步骤未执行，不能标记已通过。

Mac 构建要求 Xcode、CMake 3.22+，工作流使用 macos-14、arm64+x86_64、macOS 11.0 deployment target。
Linux 工作流使用 Ubuntu 22.04 x64，依赖和构建命令均在 YAML 内。
构建通过后脚本 scripts/package_portable.py 生成平台包及 SHA256；Mac 使用 ad-hoc 签名，不是 Developer ID 签名，也未 Apple 公证。
实际宿主监听、播放、录制、CC64 保存仍须在目标宿主验证。
