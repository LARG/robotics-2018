#ifndef __UTNAOTOOL_VISION_WINDOW_H__
#define __UTNAOTOOL_VISION_WINDOW_H__

#define DEBUG_WINDOW 0
#define MS_BETWEEN_FRAMES 1000

#include <QWidget>
#include <QApplication>

#include <map>

#include "ImageWidget.h"
#include "ui_VisionWindow.h"

#include <memory/MemoryFrame.h>
#include <VisionCore.h>
#include <vision/VisionModule.h>
#include <vision/ColorTableMethods.h>
#include <vision/structures/BallCandidate.h>

#include <vision/structures/Sample.h>

#include <vision/CameraMatrix.h>
#include <tool/ConfigWindow.h>
#include <tool/VisionConfig.h>


#define RAW_IMAGE 0
#define SEG_IMAGE 1
#define HORIZONTAL_BLOB_IMAGE 2
#define VERTICAL_BLOB_IMAGE 3
#define OBJ_IMAGE 4
#define TRANSFORMED_IMAGE 5

class RobotVisionBlock;
class ImageBlock;
class VisionObjectsBlock;
class CameraBlock;
class JointBlock;
class SensorBlock;
class WorldObjectBlock;
class BodyModelBlock;
class RobotStateBlock;
class JointCalibrator;

typedef Pose2D RobotPose;

class VisionWindow : public ConfigWindow, public Ui_UTVisionWindow {
  Q_OBJECT

  private:
    map<int,ImageProcessor*> _imageProcessors;
    map<ImageWidget*,int> _widgetAssignments;       // maps image widgets to top or bottom camera

    ImageProcessor* getImageProcessor(ImageWidget*);
    ImageProcessor* getImageProcessor(int);
    int getImageAssignment(ImageWidget*);

    VisionCore *core_;

    QMainWindow* parent_;
    int currentBigImageType_;
    int currentBigImageCam_;

    RobotVisionBlock* robot_vision_block_;
    ImageBlock* image_block_;
    VisionObjectsBlock* vision_objects_block_;
    CameraBlock* camera_block_;
    JointBlock* joint_block_;
    SensorBlock* sensor_block_;
    WorldObjectBlock* world_object_block_;
    BodyModelBlock* body_model_block_;
    RobotStateBlock* robot_state_block_;
    MemoryFrame* last_memory_;
    MemoryFrame* vision_memory_;

    QRgb segRGB[Color::NUM_Colors];
    QColor segCol[Color::NUM_Colors];
    QColor sampleColor, calibrationLineColor, connectionLineColor;

    bool doingClassification_;
    bool colorUpdateAvailable_;
    int currentY_;
    int currentU_;
    int currentV_;
    unsigned char tempTable[LUT_SIZE];
    unsigned char undoTable[LUT_SIZE];
    int undoImage_;

    JointCalibrator* jcalibrator_;
    bool doingCalibration_, checkerboard_;
    bool streaming_;

    // Tooltips
    int mouseOverBlobIndex_;
    int mouseOverBlobType_;
    int mouseOverLineIndex_;
    int mouseOverObjectIndex_;
    int mouseOverLineType_;

    void assignImageWidgets();
    void assignProcessors();
    void setImageSizes();

    bool initialized_;
    int frame_ = -1;
    LogViewer* log_ = nullptr;

    QTime timer_;
    bool enableDraw_;

    VisionConfig config_;

  public:
    VisionWindow(QMainWindow* parent, VisionCore *core);
    ~VisionWindow();

    void update(MemoryFrame* memory);
    void drawBallCands(ImageWidget* image);
    void drawBall(ImageWidget* image);
    void drawGoal(ImageWidget* image);
    void drawTransformedPoints(ImageWidget *image);
    void drawWorldObject(ImageWidget* image, QColor color, int worldObjectID);
    void drawHorizonLine(ImageWidget* image);
    void updateToolTip(int);

    void drawRawImage(ImageWidget*);
    void drawSmallSegmentedImage(ImageWidget *image);
    void drawSegmentedImage(ImageWidget *image);
    void drawBeacons(ImageWidget *image);
    void changeBigImage(int type, int cam);
    void updateBigImage();
    void updateBigImage(ImageWidget *image);

    float getBodyTilt();
    float getHeadTilt();
    float getHeadPan();
    float getBodyRoll();
    float getTrueFrontHeight() const;
    Pose3D getHeadMatrix();
    Pose3D getTorsoMatrix();

    void updateTable(unsigned char *colorTable, int yIdx, int uIdx, int vIdx);

    void newTable(Camera::Type camera);
    void openTable(Camera::Type camera);
    void writeTable(Camera::Type camera, std::string fileName);
    void saveTable(Camera::Type camera);
    void saveTableAs(Camera::Type camera);

    bool eventFilter(QObject*, QEvent*);
    void wheelEvent(QWheelEvent* event);

  public slots:

    void changeToRawTop();
    void changeToSegTop();
    void changeToHorizontalBlobTop();
    void changeToVerticalBlobTop();
    void changeToObjTop();
    void changeToTransformedTop();
    void changeToRawBottom();
    void changeToSegBottom();
    void changeToHorizontalBlobBottom();
    void changeToVerticalBlobBottom();
    void changeToObjBottom();
    void changeToTransformedBottom();

    void handleRunningCore();
    void setStreaming(bool value);


    void redrawImages();
    void redrawImages(ImageWidget* rawImage, ImageWidget* segImage, ImageWidget* objImage, ImageWidget* horizontalBlobImage, ImageWidget* verticalBlobImage, ImageWidget* transformedImage);

    void saveImages();

    void updateClassificationCheck(bool value);
    void handleClicked(int xIdx, int yIdx, Qt::MouseButton button);
    void updateCursorInfo(int x, int y);
    void updateCursorInfoVertical(int x, int y, int image);
    void updateCursorInfoHorizontal(int x, int y, int image);
    void updateCursorInfoRaw(int x, int y, int image);
    void updateCursorInfoObj(int x, int y, int image);
    void doUndo();

    void bottomNewTable();
    void bottomSaveTableAs();
    void bottomSaveTable();
    void bottomOpenTable();
    void topNewTable();
    void topSaveTableAs();
    void topSaveTable();
    void topOpenTable();

    void updateCalibrationCheck(bool value);
    void calibrationsUpdated();
    vector<LineSegment> getCalibrationLineSegments(ImageWidget*);
    Point2D getNearestLinePoint(ImageWidget*, Sample);

    void clearSamples();

    void handleNewStreamFrame();
    void handleNewLogFrame(int);
    void handleNewLogLoaded(LogViewer*);

    void update();

    void loadConfig(const ToolConfig& config) final;
    void saveConfig(ToolConfig& config) final;
    void controlsChanged() final;


  signals:
    void processFrame();
    void setCore(bool value);
    void prevSnapshot();
    void nextSnapshot();
    void gotoSnapshot(int frame);
    void play();
    void pause();
    void newStreamFrame();
    void newLogFrame(int);
    void newLogLoaded(LogViewer*);
    void cameraChanged(Camera::Type);
    void colorTableLoaded();
    void calibrationSampleAdded(Sample s);

  protected:
    void showEvent(QShowEvent* event) override;
};

#endif
