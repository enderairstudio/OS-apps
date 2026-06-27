#ifndef SYSTEMINFO_H
#define SYSTEMINFO_H

#include <QString>
#include <QVector>
#include <QDateTime>

struct HardwareInfo {
    QString cpuModel;
    QString gpuModel;
    QString ramSize;
    QString motherboard;
    QString sysModel;
};

struct DiskInfo {
    QString mountPoint;
    double usedGiB;
    double totalGiB;
    double percentage;
};

struct NetworkInfo {
    QString ssid;
    QString ipv4;
    double downloadSpeed; // in Bytes per second
    double uploadSpeed;   // in Bytes per second
};

class SystemInfo {
public:
    SystemInfo();
    ~SystemInfo();

    // Call this once per tick to compute diffs (e.g. CPU & Network speed)
    void update();

    double getCpuUsage() const;
    double getGpuUsage() const;
    void getMemoryUsage(double &usedGiB, double &totalGiB) const;
    void getSwapUsage(double &usedBytes, double &totalGiB) const;
    
    HardwareInfo getHardwareInfo() const;
    QVector<DiskInfo> getDiskInfo() const;
    NetworkInfo getNetworkInfo() const;

private:
    // CPU sampling state
    unsigned long long lastUser, lastNice, lastSystem, lastIdle, lastIowait, lastIrq, lastSoftirq, lastSteal;
    double currentCpuUsage;

    // Network sampling state
    unsigned long long lastRxBytes, lastTxBytes;
    QDateTime lastNetworkTime;
    QString activeInterface;
    double currentDownloadSpeed;
    double currentUploadSpeed;

    // Hardware static info cached
    HardwareInfo cachedHardwareInfo;

    // Helpers
    void readCpuStats(unsigned long long &user, unsigned long long &nice, unsigned long long &system,
                      unsigned long long &idle, unsigned long long &iowait, unsigned long long &irq,
                      unsigned long long &softirq, unsigned long long &steal) const;
    void detectActiveNetworkInterface();
    void readNetworkBytes(unsigned long long &rx, unsigned long long &tx) const;
    void initHardwareInfo();
    
    // GPU loading helpers
    double queryIntelGpuUsage() const;
    double queryAmdGpuUsage() const;
    double queryNvidiaGpuUsage() const;
    QString detectGpuModel() const;
};

#endif // SYSTEMINFO_H
