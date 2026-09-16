import java.io.File

plugins {
    id("com.android.application")
}

android {
    namespace = "com.example.opengltest"
    compileSdk = 34
    ndkVersion = "26.1.10909125"

    defaultConfig {
        applicationId = "com.example.opengltest"
        minSdk = 24
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"

        ndk {
            abiFilters += listOf("arm64-v8a", "armeabi-v7a")
        }

        externalNativeBuild {
            cmake {
                cppFlags += "-std=c++17"
                arguments += "-DANDROID_STL=c++_shared"
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("../../CMakeLists.txt")
            version = "3.22.1"
        }
    }

    sourceSets {
        getByName("main") {
            // Populated by the prepareAssets task below (copies data/ + writes
            // assets_manifest.txt, both consumed by game/android_main.cpp).
            assets.srcDirs("build/generatedAssets")
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }
}

// Copies the repo-root data/ folder into the APK assets and generates
// assets_manifest.txt (a flat list of every asset path) since Android's
// AAssetManager cannot enumerate asset subdirectories at runtime - the
// native code needs the manifest to know what to extract on first launch.
tasks.register("prepareAssets") {
    doLast {
        val dataDir = file("${projectDir}/../../data")
        val outDir = file("${projectDir}/build/generatedAssets")
        val outDataDir = File(outDir, "data")

        outDataDir.deleteRecursively()
        outDataDir.mkdirs()
        dataDir.copyRecursively(outDataDir, overwrite = true)

        val entries = outDataDir.walkTopDown()
            .filter { it.isFile }
            .map { outDir.toPath().relativize(it.toPath()).toString().replace('\\', '/') }
            .sorted()
            .toList()

        File(outDir, "assets_manifest.txt").writeText(entries.joinToString("\n"))
    }
}

tasks.named("preBuild") {
    dependsOn("prepareAssets")
}
