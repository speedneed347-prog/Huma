# HUMA X — মোবাইলে চালানোর সম্পূর্ণ নির্দেশিকা (Mobile Guide)

HUMA X (Human-Oriented Universal Machine Language) আপনি আপনার মোবাইল ফোনে **২টি উপায়ে** খুব সহজে চালাতে পারবেন:

---

## পদ্ধতি ১: অফলাইন মোবাইল ওয়েব রানার (সবচেয়ে সহজ - কোনো ইনস্টলেশন লাগবে না)

১. `huma-mobile.html` ফাইলটি ডাউনলোড করে আপনার মোবাইলের Chrome, Safari বা যেকোনো ব্রাউজারে ওপেন করুন।
২. এটি **১০০% অফলাইনে** কাজ করে—ইন্টারনেট বা ওয়াইফাই বন্ধ থাকলেও আপনি সম্পূর্ণ কোড লিখতে ও চালাতে পারবেন।
৩. এতে মোবাইল ফ্রেন্ডলি ভার্চুয়াল কিবোর্ড শর্টকাট (`make`, `fixed`, `show`, `if`, `end`, `repeat`) যুক্ত আছে।

---

## পদ্ধতি ২: Android মোবাইলে Termux দিয়ে আসল কম্পাইলার চালানো (Native CLI)

১. আপনার মোবাইলে **F-Droid** বা Google Play Store থেকে **Termux** অ্যাপ ইনস্টল করুন।
২. Termux ওপেন করে নিচের কমান্ডটি কপি করে পেস্ট করুন:
```bash
pkg update && pkg install -y clang cmake make git
git clone https://github.com/huma-lang/huma-x.git
cd huma-x/huma_lang
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j4
cp build/huma $PREFIX/bin/
```
৩. এবার আপনার মোবাইলের Termux-এ সরাসরি টেস্ট করুন:
```bash
huma version
huma run examples/hello.huma
huma repl
```

---

## GitHub Actions Release Download

GitHub Actions ওয়ার্কফ্লো (`.github/workflows/build-and-release.yml`) স্বয়ংক্রিয়ভাবে তৈরি করে:
- `huma-android-arm64`: Android ARM64 ডিভাইসের জন্য কম্পাইল করা বাইনারি
- `huma-linux-x86_64`: Linux বাইনারি
- `huma-windows-x64.exe`: Windows ইনস্টলার
- `huma-macos-universal`: Mac বাইনারি
- `huma-mobile-offline-runner.html`: মোবাইল ব্রাউজার অফলাইন ফাইল
