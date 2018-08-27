#pragma once

class ConfigBase {
  public:
    virtual void controlsChanged(const QString&) = 0;
    virtual void controlsChanged(double) = 0;
    virtual void controlsChanged(float) = 0;
    virtual void controlsChanged(int) = 0;
    virtual void controlsChanged(short) = 0;
    virtual void controlsChanged(char) = 0;
    virtual void controlsChanged(bool) = 0;
};
