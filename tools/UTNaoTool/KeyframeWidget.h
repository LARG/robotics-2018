#pragma once

#include <QtGui>
#include <QWidget>

#include <tool/ConfigWidget.h>
#include <tool/KeyframeConfig.h>
#include <memory/MemoryCache.h>
#include <common/Keyframe.h>
#include "ui_KeyframeWidget.h"
#include "ui_KeyframeItem.h"

class KeyframeItem : public QWidget, public Ui_KeyframeItem {
  Q_OBJECT
  public:
    KeyframeItem(QListWidget* parent, Keyframe keyframe, int row = -1) : QWidget(parent), keyframe_(keyframe) {
      setupUi(this);
      init(row);
    }
    const Keyframe& keyframe() { return keyframe_; }
    QListWidgetItem* item() { return item_; }
    void init(int row);
  public slots:
    void updateName();
    void updateFrames(int frames);
    void activate();
    void deactivate();
    void moveUp();
    void moveDown();
  protected:
    void updateKeyframe(const Keyframe& kf);
    void swap(QListWidgetItem* oitem);
    QListWidgetItem* createParentItem(int row);
  private:
    Keyframe keyframe_;
    QListWidgetItem* item_;
    QListWidget* list_;
};

class KeyframeWidget : public ConfigWidget, public Ui_KeyframeWidget {
  Q_OBJECT
  public:
    KeyframeWidget(QWidget* parent);
    void updateMemory(MemoryCache cache);

  signals:
    void playingSequence(const Keyframe& start, const Keyframe& finish, int cframe);
    void showingKeyframe(const Keyframe& keyframe);
    void updatedSupportBase(SupportBase base);

  protected:
    void loadConfig(const ToolConfig& config);
    void saveConfig(ToolConfig& config);

  protected slots:
    void save();
    void reload();
    void saveAs();
    void load();
    void addKeyframe();
    void deleteKeyframe();
    void updateItem(QListWidgetItem* item);
    void play();
    void playNextFrame();
    void show();
    void activate(QListWidgetItem *item);
    void deactivateCurrent();
    void supportBaseUpdated(bool);

  private:
    int currentKeyframe_, currentFrame_;
    MemoryCache cache_;
    QTimer* keyframeTimer_;
    KeyframeItem* activated_;
    KeyframeConfig kfconfig_;
};

