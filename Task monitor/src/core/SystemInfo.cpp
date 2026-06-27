#include "SystemInfo.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

SystemInfo::SystemInfo()
    : lastUser(0), lastNice(0), lastSystem(0), lastIdle(0), lastIowait(0),
      lastIrq(0), lastSoftirq(0), lastSteal(0), currentCpuUsage(0.0),
      lastRxBytes(0), lastTxBytes(0), currentDownloadSpeed(0.0), currentUploadSpeed(0.0)
{
    lastNetworkTime = QDateTime::currentDateTime();
    
    // Initialize samples
    readCpuStats(lastUser, lastNice, lastSystem, lastIdle, lastIowait, lastIrq, lastSoftirq, lastSteal);
    detectActiveNetworkInterface();
    readNetworkBytes(lastRxBytes, lastTxBytes);
    
    // Initialize static hardware info
    initHardwareInfo();
}

SystemInfo::~SystemInfo() {}

void SystemInfo::readCpuStats(unsigned long long &user, unsigned long long &nice, unsigned long long &system,
                               unsigned long long &idle, unsigned long long &iowait, unsigned long long &irq,
                               unsigned long long &softirq, unsigned long long &steal) const {
    std::ifstream file("/proc/stat");
    if (!file.is_open()) return;

    std::string line;
    if (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string cpu;
        ss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;
    }
}

void SystemInfo::detectActiveNetworkInterface() {
    // Check if we can find default route interface
    std::ifstream file("/proc/net/route");
    if (file.is_open()) {
        std::string line;
        // Skip header
        std::getline(file, line);
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string iface;
            std::string dest;
            ss >> iface >> dest;
            if (dest == "00000000") { // Default route
                activeInterface = QString::fromStdString(iface);
                return;
            }
        }
    }
    
    // Fallback: search /proc/net/dev for first interface that isn't lo
    std::ifstream devFile("/proc/net/dev");
    if (devFile.is_open()) {
        std::string line;
        // Skip header lines
        std::getline(devFile, line);
        std::getline(devFile, line);
        while (std::getline(devFile, line)) {
            size_t colon = line.find(':');
            if (colon != std::string::npos) {
                std::string iface = line.substr(0, colon);
                // Strip whitespace
                iface.erase(0, iface.find_first_not_of(" \t"));
                iface.erase(iface.find_last_not_of(" \t") + 1);
                if (iface != "lo" && !iface.empty()) {
                    activeInterface = QString::fromStdString(iface);
                    return;
                }
            }
        }
    }
    activeInterface = "wlan0"; // Default fallback
}

void SystemInfo::readNetworkBytes(unsigned long long &rx, unsigned long long &tx) const {
    rx = 0;
    tx = 0;
    if (activeInterface.isEmpty()) return;

    std::ifstream file("/proc/net/dev");
    if (!file.is_open()) return;

    std::string line;
    std::string targetIface = activeInterface.toStdString();
    
    // Skip 2 header lines
    std::getline(file, line);
    std::getline(file, line);
    
    while (std::getline(file, line)) {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string iface = line.substr(0, colon);
            iface.erase(0, iface.find_first_not_of(" \t"));
            iface.erase(iface.find_last_not_of(" \t") + 1);
            
            if (iface == targetIface) {
                std::string data = line.substr(colon + 1);
                std::istringstream ss(data);
                unsigned long long dummy;
                // Format: rx_bytes rx_packets rx_errs rx_drop rx_fifo rx_frame rx_compressed rx_multicast tx_bytes ...
                ss >> rx; // first column after colon is rx bytes
                for (int i = 0; i < 7; ++i) ss >> dummy;
                ss >> tx; // 9th column after colon is tx bytes
                break;
            }
        }
    }
}

void SystemInfo::update() {
    // 1. Update CPU
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    readCpuStats(user, nice, system, idle, iowait, irq, softirq, steal);

    unsigned long long prevIdle = lastIdle + lastIowait;
    unsigned long long currIdle = idle + iowait;

    unsigned long long prevNonIdle = lastUser + lastNice + lastSystem + lastIrq + lastSoftirq + lastSteal;
    unsigned long long currNonIdle = user + nice + system + irq + softirq + steal;

    unsigned long long prevTotal = prevIdle + prevNonIdle;
    unsigned long long currTotal = currIdle + currNonIdle;

    unsigned long long totalDiff = currTotal - prevTotal;
    unsigned long long idleDiff = currIdle - prevIdle;

    if (totalDiff > 0) {
        currentCpuUsage = (double)(totalDiff - idleDiff) / totalDiff * 100.0;
    } else {
        currentCpuUsage = 0.0;
    }

    lastUser = user;
    lastNice = nice;
    lastSystem = system;
    lastIdle = idle;
    lastIowait = iowait;
    lastIrq = irq;
    lastSoftirq = softirq;
    lastSteal = steal;

    // 2. Update Network
    QDateTime now = QDateTime::currentDateTime();
    double elapsedSeconds = lastNetworkTime.msecsTo(now) / 1000.0;
    if (elapsedSeconds < 0.1) elapsedSeconds = 1.0;

    unsigned long long rx, tx;
    readNetworkBytes(rx, tx);

    if (lastRxBytes > 0 && rx >= lastRxBytes) {
        currentDownloadSpeed = (double)(rx - lastRxBytes) / elapsedSeconds;
    } else {
        currentDownloadSpeed = 0.0;
    }

    if (lastTxBytes > 0 && tx >= lastTxBytes) {
        currentUploadSpeed = (double)(tx - lastTxBytes) / elapsedSeconds;
    } else {
        currentUploadSpeed = 0.0;
    }

    lastRxBytes = rx;
    lastTxBytes = tx;
    lastNetworkTime = now;
}

double SystemInfo::getCpuUsage() const {
    if (currentCpuUsage < 0.0) return 0.0;
    if (currentCpuUsage > 100.0) return 100.0;
    return currentCpuUsage;
}

double SystemInfo::queryIntelGpuUsage() const {
    // Scan DRM card paths for gt frequency
    // Check if card1 or card0 contains gt_act_freq_mhz and gt_max_freq_mhz
    QStringList paths = {
        "/sys/class/drm/card1/device/drm/card1",
        "/sys/class/drm/card0/device/drm/card0",
        "/sys/class/drm/card1/device",
        "/sys/class/drm/card0/device"
    };

    for (const QString &basePath : paths) {
        QFile actFile(basePath + "/gt_act_freq_mhz");
        QFile maxFile(basePath + "/gt_max_freq_mhz");
        if (actFile.open(QIODevice::ReadOnly) && maxFile.open(QIODevice::ReadOnly)) {
            double act = QString(actFile.readAll()).trimmed().toDouble();
            double max = QString(maxFile.readAll()).trimmed().toDouble();
            actFile.close();
            maxFile.close();
            if (max > 0) {
                return (act / max) * 100.0;
            }
        }
    }
    
    // Also try GT/GT0 path
    QFile gt0Act("/sys/class/drm/card1/device/drm/card1/gt/gt0/rps_act_freq_mhz");
    QFile gt0Max("/sys/class/drm/card1/device/drm/card1/gt/gt0/rps_max_freq_mhz");
    if (gt0Act.open(QIODevice::ReadOnly) && gt0Max.open(QIODevice::ReadOnly)) {
        double act = QString(gt0Act.readAll()).trimmed().toDouble();
        double max = QString(gt0Max.readAll()).trimmed().toDouble();
        gt0Act.close();
        gt0Max.close();
        if (max > 0) {
            return (act / max) * 100.0;
        }
    }
    return -1.0;
}

double SystemInfo::queryAmdGpuUsage() const {
    QDir drmDir("/sys/class/drm");
    if (!drmDir.exists()) return -1.0;
    
    QStringList entries = drmDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        if (entry.startsWith("card")) {
            QFile busyFile("/sys/class/drm/" + entry + "/device/gpu_busy_percent");
            if (busyFile.open(QIODevice::ReadOnly)) {
                double usage = QString(busyFile.readAll()).trimmed().toDouble();
                busyFile.close();
                return usage;
            }
        }
    }
    return -1.0;
}

double SystemInfo::queryNvidiaGpuUsage() const {
    QProcess proc;
    proc.start("nvidia-smi", QStringList() << "--query-gpu=utilization.gpu" << "--format=csv,noheader,nounits");
    if (proc.waitForFinished(200)) {
        QString out = QString(proc.readAllStandardOutput()).trimmed();
        bool ok;
        double val = out.toDouble(&ok);
        if (ok) return val;
    }
    return -1.0;
}

double SystemInfo::getGpuUsage() const {
    double usage = queryNvidiaGpuUsage();
    if (usage >= 0.0) return usage;
    
    usage = queryAmdGpuUsage();
    if (usage >= 0.0) return usage;
    
    usage = queryIntelGpuUsage();
    if (usage >= 0.0) return usage;
    
    return 0.0;
}

void SystemInfo::getMemoryUsage(double &usedGiB, double &totalGiB) const {
    usedGiB = 0.0;
    totalGiB = 0.0;

    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) return;

    std::string line;
    unsigned long long memTotal = 0;
    unsigned long long memAvailable = 0;
    bool foundTotal = false, foundAvail = false;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string key;
        unsigned long long val;
        ss >> key >> val;
        if (key == "MemTotal:") {
            memTotal = val;
            foundTotal = true;
        } else if (key == "MemAvailable:") {
            memAvailable = val;
            foundAvail = true;
        }
        if (foundTotal && foundAvail) break;
    }

    if (foundTotal) {
        totalGiB = (double)memTotal / (1024.0 * 1024.0);
        if (foundAvail) {
            usedGiB = (double)(memTotal - memAvailable) / (1024.0 * 1024.0);
        } else {
            usedGiB = totalGiB;
        }
    }
}

void SystemInfo::getSwapUsage(double &usedBytes, double &totalGiB) const {
    usedBytes = 0.0;
    totalGiB = 0.0;

    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) return;

    std::string line;
    unsigned long long swapTotal = 0;
    unsigned long long swapFree = 0;
    bool foundTotal = false, foundFree = false;

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string key;
        unsigned long long val;
        ss >> key >> val;
        if (key == "SwapTotal:") {
            swapTotal = val;
            foundTotal = true;
        } else if (key == "SwapFree:") {
            swapFree = val;
            foundFree = true;
        }
        if (foundTotal && foundFree) break;
    }

    if (foundTotal) {
        totalGiB = (double)swapTotal / (1024.0 * 1024.0);
        if (foundFree) {
            usedBytes = (double)(swapTotal - swapFree) * 1024.0; // In bytes
        }
    }
}

QString SystemInfo::detectGpuModel() const {
    QProcess proc;
    proc.start("lspci", QStringList());
    if (proc.waitForFinished(1000)) {
        QString out = QString(proc.readAllStandardOutput());
        QStringList lines = out.split("\n");
        for (const QString &line : lines) {
            if (line.contains("VGA compatible controller", Qt::CaseInsensitive) ||
                line.contains("3D controller", Qt::CaseInsensitive) ||
                line.contains("Display controller", Qt::CaseInsensitive)) {
                
                // Get the hardware description part after the colon
                int lastColon = line.lastIndexOf(':');
                if (lastColon != -1 && lastColon + 1 < line.length()) {
                    QString model = line.mid(lastColon + 1).trimmed();
                    // Clean up common long prefixes
                    model.remove("Intel Corporation ");
                    model.remove("Advanced Micro Devices, Inc. ");
                    model.remove("NVIDIA Corporation ");
                    return model;
                }
            }
        }
    }
    return "Intel HD Graphics";
}

void SystemInfo::initHardwareInfo() {
    // 1. CPU Model
    cachedHardwareInfo.cpuModel = "Unknown CPU";
    std::ifstream cpuFile("/proc/cpuinfo");
    if (cpuFile.is_open()) {
        std::string line;
        while (std::getline(cpuFile, line)) {
            if (line.rfind("model name", 0) == 0) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string model = line.substr(colon + 1);
                    // Trim whitespace
                    model.erase(0, model.find_first_not_of(" \t"));
                    // Clean up names a bit for better UI layout
                    size_t pos;
                    while ((pos = model.find("(R)")) != std::string::npos) model.erase(pos, 3);
                    while ((pos = model.find("(TM)")) != std::string::npos) model.erase(pos, 4);
                    cachedHardwareInfo.cpuModel = QString::fromStdString(model).trimmed();
                    break;
                }
            }
        }
    }

    // 2. GPU Model
    cachedHardwareInfo.gpuModel = detectGpuModel();

    // 3. RAM Size
    double used, total;
    getMemoryUsage(used, total);
    cachedHardwareInfo.ramSize = QString("%1 GB").arg(qRound(total));

    // 4. Motherboard vendor & name
    QFile mbVendorFile("/sys/class/dmi/id/board_vendor");
    QFile mbNameFile("/sys/class/dmi/id/board_name");
    QString vendor = "Unknown";
    QString name = "Motherboard";
    if (mbVendorFile.open(QIODevice::ReadOnly)) {
        vendor = QString(mbVendorFile.readAll()).trimmed();
        mbVendorFile.close();
    }
    if (mbNameFile.open(QIODevice::ReadOnly)) {
        name = QString(mbNameFile.readAll()).trimmed();
        mbNameFile.close();
    }
    if (vendor == "Unknown" && name == "Motherboard") {
        cachedHardwareInfo.motherboard = "Standard Mainboard";
    } else {
        cachedHardwareInfo.motherboard = vendor + " " + name;
    }

    // 5. System model
    QFile sysVendorFile("/sys/class/dmi/id/sys_vendor");
    QFile sysProductFile("/sys/class/dmi/id/product_name");
    QString sysVendor = "";
    QString sysProduct = "";
    if (sysVendorFile.open(QIODevice::ReadOnly)) {
        sysVendor = QString(sysVendorFile.readAll()).trimmed();
        sysVendorFile.close();
    }
    if (sysProductFile.open(QIODevice::ReadOnly)) {
        sysProduct = QString(sysProductFile.readAll()).trimmed();
        sysProductFile.close();
    }
    if (!sysVendor.isEmpty() || !sysProduct.isEmpty()) {
        cachedHardwareInfo.sysModel = (sysVendor + " " + sysProduct).trimmed();
    } else {
        cachedHardwareInfo.sysModel = "Linux PC";
    }
}

HardwareInfo SystemInfo::getHardwareInfo() const {
    return cachedHardwareInfo;
}

QVector<DiskInfo> SystemInfo::getDiskInfo() const {
    QVector<DiskInfo> disks;
    
    // We will query "/" (root disk) and optionally "/home" if it's on a different partition
    QStringList paths = {"/"};
    
    // Check if "/home" is a separate mount point
    std::ifstream file("/proc/mounts");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream ss(line);
            std::string spec, fileMount, vfstype;
            ss >> spec >> fileMount >> vfstype;
            if (fileMount == "/home") {
                paths.append("/home");
                break;
            }
        }
    }

    for (const QString &path : paths) {
        struct statvfs buf;
        if (statvfs(path.toStdString().c_str(), &buf) == 0) {
            double total = (double)buf.f_blocks * buf.f_frsize / (1024.0 * 1024.0 * 1024.0);
            double free = (double)buf.f_bavail * buf.f_frsize / (1024.0 * 1024.0 * 1024.0);
            double used = total - free;
            double percentage = (total > 0) ? (used / total) * 100.0 : 0.0;
            
            DiskInfo info;
            info.mountPoint = path;
            info.usedGiB = used;
            info.totalGiB = total;
            info.percentage = percentage;
            disks.append(info);
        }
    }
    return disks;
}

NetworkInfo SystemInfo::getNetworkInfo() const {
    NetworkInfo info;
    info.downloadSpeed = currentDownloadSpeed;
    info.uploadSpeed = currentUploadSpeed;
    info.ssid = "Disconnected";
    info.ipv4 = "127.0.0.1";

    // 1. Get Wifi SSID
    QProcess proc;
    proc.start("nmcli", QStringList() << "-t" << "-f" << "ACTIVE,SSID" << "dev" << "wifi");
    if (proc.waitForFinished(1500)) {
        QString out = QString(proc.readAllStandardOutput());
        QStringList lines = out.split("\n");
        for (const QString &line : lines) {
            if (line.startsWith("yes:")) {
                info.ssid = line.mid(4).trimmed();
                if (info.ssid.isEmpty()) {
                    info.ssid = "Hidden Network";
                }
                break;
            }
        }
    }
    
    // Fallback: If Wifi is disconnected but we have active network, check if ethernet/other
    if (info.ssid == "Disconnected") {
        if (!activeInterface.isEmpty() && activeInterface != "lo") {
            if (activeInterface.startsWith("en") || activeInterface.startsWith("eth")) {
                info.ssid = "Wired Connection (" + activeInterface + ")";
            } else {
                info.ssid = "Active Network (" + activeInterface + ")";
            }
        }
    }

    // 2. Get IPv4 address of active interface
    struct ifaddrs *ifAddrStruct = nullptr;
    struct ifaddrs *ifa = nullptr;
    void *tmpAddrPtr = nullptr;

    if (getifaddrs(&ifAddrStruct) == 0) {
        for (ifa = ifAddrStruct; ifa != nullptr; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr) continue;
            
            if (ifa->ifa_addr->sa_family == AF_INET) { // Is IPv4
                tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                char addressBuffer[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
                
                QString ifName = QString::fromStdString(ifa->ifa_name);
                if (ifName == activeInterface) {
                    info.ipv4 = QString(addressBuffer);
                    break;
                }
                // Fallback to first non-loopback if match with activeInterface fails
                if (ifName != "lo" && info.ipv4 == "127.0.0.1") {
                    info.ipv4 = QString(addressBuffer);
                }
            }
        }
        if (ifAddrStruct) freeifaddrs(ifAddrStruct);
    }

    return info;
}
