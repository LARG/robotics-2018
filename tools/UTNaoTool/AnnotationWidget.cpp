#include "AnnotationWidget.h"
#include <common/annotations/SelectionType.h>
#include <iostream>

AnnotationWidget::AnnotationWidget(QWidget* parent) : QWidget(parent), selectedAnnotation_(0) {
    setupUi(this);

    colorStrings[c_UNDEFINED]="Undefined";
    colorStrings[c_FIELD_GREEN]="Field Green";
    colorStrings[c_WHITE]="White";
    colorStrings[c_ORANGE]="Orange";
    colorStrings[c_PINK]="Pink";
    colorStrings[c_BLUE]="Blue";
    colorStrings[c_YELLOW]="Yellow";
    colorStrings[c_ROBOT_WHITE] = "Robot White";

    for (int i=0; i<NUM_Colors; i++) {
        colorBox->addItem(colorStrings[i]);
    }

    selectionStrings[SelectionTypeMethods::index(SelectionType::Rectangle)] = "Rectangle";
    selectionStrings[SelectionTypeMethods::index(SelectionType::Polygon)] = "Polygon";
    selectionStrings[SelectionTypeMethods::index(SelectionType::Ellipse)] = "Ellipse";
    selectionStrings[SelectionTypeMethods::index(SelectionType::Manual)] = "Manual";

    for(int i = 0; i < SelectionTypeMethods::NumSelectionTypes(); i++)
        selectionTypeBox->addItem(selectionStrings[i]);

    cameraStrings[Camera::TOP] = "Top";
    cameraStrings[Camera::BOTTOM] = "Bottom";

    cameraBox->addItem(cameraStrings[0]);
    cameraBox->addItem(cameraStrings[1]);

    connect (annotationList, SIGNAL(itemSelectionChanged()), this, SLOT(annotationSelected()));
    connect (selectionTypeBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(selectionBoxIndexChanged(const QString &)));
    connect (cameraBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(cameraBoxIndexChanged(const QString &)));
    connect (colorBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(colorBoxIndexChanged(const QString &)));
    connect (checkBox, SIGNAL(stateChanged(int)), this, SLOT(annotationModeChanged(int)));

    //connect (insertButton, SIGNAL(clicked()), this, SLOT(insert()));
    connect (updateButton, SIGNAL(clicked()), this, SLOT(update()));
    connect (deleteAnnotButton, SIGNAL(clicked()), this, SLOT(deleteAnnotation()));
    connect (deleteSelectionButton, SIGNAL(clicked()), this, SLOT(deleteSelection()));

    connect (greenBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));
    connect (orangeBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));
    connect (yellowBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));
    connect (whiteBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));
    connect (robotWhiteBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));
    connect (pinkBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));
    connect (blueBox, SIGNAL(clicked()), this, SLOT(redrawCurrentSelections()));

    connect (saveButton, SIGNAL(clicked()), this, SLOT(saveToFile()));

    connect (defineMovementsButton, SIGNAL(clicked()), this, SLOT(toggleDefiningMovements()));
    connect (clearMovementsButton, SIGNAL(clicked()), this, SLOT(clearMovements()));

    currentCamera_ = selectedCamera_ = Camera::TOP;
    definingMovements_ = false;
}

void AnnotationWidget::setMaxFrames(int frames){
    maxFrames_ = frames;
    minBox->setMaximum(frames - 1);
    maxBox->setMaximum(frames - 1);
    maxBox->setValue(frames - 1);
}

void AnnotationWidget::selected(Selection* selection){
    std::string name = annotname->text().toStdString();
    VisionAnnotation* annotation;
    std::map<std::string, VisionAnnotation* >::iterator it = annotations_.find(name);
    if(it != annotations_.end())
        annotation = it->second;
    else {
        annotation = new VisionAnnotation(name);
        annotations_[annotation->getName()] = annotation;
        annotation->setMaxFrame(maxBox->value());
        annotation->setMinFrame(minBox->value());
        annotation->setColor(selectedColor_);
        annotation->setCamera(selectedCamera_);
        annotation->setSample(cbxSampleSet->isChecked());
        AnnotationListWidgetItem* item = new AnnotationListWidgetItem(annotation);
        annotationList->addItem(item);
        if(!selectedAnnotation_) {
            selectedAnnotation_ = annotation;
            annotationList->setCurrentItem(item);
        }
    }

    annotation->addSelection(selection);
    if(selectedAnnotation_ == annotation)
        loadSelections(annotation);
    emit setCurrentAnnotations(getAnnotations());
    redrawCurrentSelections();
}

std::vector<VisionAnnotation*> AnnotationWidget::getAnnotations() {
    std::vector<VisionAnnotation*> annotations;
    int count = annotationList->count();
    for(int i = 0; i < count; i++){
        QListWidgetItem* item = annotationList->item(i);
        AnnotationListWidgetItem* aitem = (AnnotationListWidgetItem*)item;
        VisionAnnotation* annotation = aitem->getAnnotation();
        annotations.push_back(annotation);
    }
    return annotations;
}

std::vector<Selection*> AnnotationWidget::getSelections() {
    std::vector<Selection*> selections;
    int count = selectionList->count();
    for(int i = 0; i < count; i++){
        QListWidgetItem* item = selectionList->item(i);
        SelectionListWidgetItem* sitem = (SelectionListWidgetItem*)item;
        Selection* selection = sitem->getSelection();
        selections.push_back(selection);
    }
    return selections;
}

void AnnotationWidget::annotationSelected(){
    if(annotationList->selectedItems().count() == 0) {
        selectionList->clear();
        clearSelections();
        return;
    }
    QListWidgetItem* item = annotationList->selectedItems()[0];
    AnnotationListWidgetItem* aitem = (AnnotationListWidgetItem*)item;
    VisionAnnotation* annotation = aitem->getAnnotation();
    loadSelections(annotation);
    loadChoices(annotation);
    selectedAnnotation_ = annotation;
}

void AnnotationWidget::loadChoices(VisionAnnotation* annotation) {
    annotname->setText(QString::fromStdString(annotation->getName()));
    colorBox->setCurrentIndex(annotation->getColor());
    cameraBox->setCurrentIndex(annotation->getCamera());
    minBox->setValue(annotation->getMinFrame());
    maxBox->setValue(annotation->getMaxFrame());
    cbxSampleSet->setChecked(annotation->isSample());
}

void AnnotationWidget::loadSelections(VisionAnnotation* annotation){
    selectionList->clear();
    const std::vector<Selection*> selections = annotation->getSelections();
    for(uint16_t i=0; i<selections.size(); i++)
        selectionList->addItem(new SelectionListWidgetItem(selections[i]));
}

void AnnotationWidget::clearSelections() {
    emit setCurrentSelections(std::vector<Selection*>());
}

void AnnotationWidget::colorBoxIndexChanged(const QString& text) {
    (void)text; // kill warning
    int index = colorBox->currentIndex();
    selectedColor_ = (Color)index;
}

void AnnotationWidget::cameraBoxIndexChanged(const QString& text) {
    std::string str = text.toStdString();
    if(str == "Top")
        selectedCamera_ = Camera::TOP;
    else
        selectedCamera_ = Camera::BOTTOM;
}


void AnnotationWidget::selectionBoxIndexChanged(const QString& text) {
    if(text == "Rectangle")
        emit selectionTypeChanged(SelectionType::Rectangle);
    else if (text == "Polygon")
        emit selectionTypeChanged(SelectionType::Polygon);
    else if (text == "Ellipse")
        emit selectionTypeChanged(SelectionType::Ellipse);
    else if (text == "Manual")
        emit selectionTypeChanged(SelectionType::Manual);
}

void AnnotationWidget::annotationModeChanged(int state){
    switch(state){
        case 0: emit selectionEnabled(false); break;
        case 1: break;
        case 2: emit selectionEnabled(true); break;
    }
}

void AnnotationWidget::handleNewLogFrame(int frame){
    currentFrame_ = frame;
    std::vector<Selection*> currentSelections;
    int count = annotationList->count();
    for(int i = 0; i < count; i++){
        QListWidgetItem* item = annotationList->item(i);
        AnnotationListWidgetItem* aitem = (AnnotationListWidgetItem*)item;
        VisionAnnotation* annotation = aitem->getAnnotation();
        annotation->setCurrentFrame(currentFrame_);
        if(annotation->getCamera() != currentCamera_) continue;
        if(!annotation->isInFrame(frame)) continue;
        if(filterAnnotation(annotation)) continue;
        const std::vector<Selection*> selections = annotation->getSelections();
        int scount = selections.size();
        for(int j=0; j < scount; j++) {
            currentSelections.push_back(selections[j]);
        }
    }
    emit setCurrentSelections(currentSelections);
}

void AnnotationWidget::setImageProcessors(ImageProcessor* top, ImageProcessor* bottom){
    topProcessor_ = top;
    bottomProcessor_ = bottom;
}

void AnnotationWidget::setCurrentCamera(Camera::Type camera){
    currentCamera_ = camera;
    cameraBox->setCurrentIndex(camera == Camera::TOP ? 0 : 1);
    redrawCurrentSelections();
}

void AnnotationWidget::handleNewLogLoaded(LogViewer* log){
    log_ = log;
    setMaxFrames(log_->size());
    annotations_.clear();
    annotationList->clear();

    AnnotationGroup group;
    if(!group.load(log)) {
        redrawCurrentSelections();
        return;
    }
    std::vector<VisionAnnotation*> annotations = group.getVisionAnnotations();
    for(uint16_t i = 0; i < annotations.size(); i++){
        VisionAnnotation* a = annotations[i];
        AnnotationListWidgetItem* item = new AnnotationListWidgetItem(a);
        annotationList->addItem(item);
        annotations_[a->getName()] = a;
    }
    redrawCurrentSelections();
    emit setCurrentAnnotations(getAnnotations());
}

void AnnotationWidget::insert(){

}

void AnnotationWidget::update(){
    if(annotationList->selectedItems().count() == 0)
        return;
    QListWidgetItem* item = annotationList->selectedItems()[0];
    AnnotationListWidgetItem* aitem = (AnnotationListWidgetItem*)item;
    VisionAnnotation* annotation = aitem->getAnnotation();
    annotation->setName(annotname->text().toStdString());
    annotation->setMaxFrame(maxBox->value());
    annotation->setMinFrame(minBox->value());
    annotation->setColor(selectedColor_);
    annotation->setCamera(selectedCamera_);
    annotation->setSample(cbxSampleSet->isChecked());
    aitem->resetText();
    redrawCurrentSelections();
}

void AnnotationWidget::deleteAnnotation(){
    if(annotationList->selectedItems().count() == 0)
        return;
    QListWidgetItem* item = annotationList->selectedItems()[0];
    AnnotationListWidgetItem* aitem = (AnnotationListWidgetItem*)item;
    VisionAnnotation* annotation = aitem->getAnnotation();
    std::map<std::string, VisionAnnotation*>::iterator it = annotations_.find(annotation->getName());
    annotations_.erase(it, annotations_.end());
    delete item;
    delete annotation;
    redrawCurrentSelections();
}

void AnnotationWidget::deleteSelection(){
    if(annotationList->selectedItems().count() == 0)
        return;
    if(selectionList->selectedItems().count() == 0)
        return;
    QListWidgetItem* item = annotationList->selectedItems()[0];
    AnnotationListWidgetItem* aitem = (AnnotationListWidgetItem*)item;
    VisionAnnotation* annotation = aitem->getAnnotation();

    QListWidgetItem* item2 = selectionList->selectedItems()[0];
    SelectionListWidgetItem* sitem = (SelectionListWidgetItem*)item2;
    Selection* selection = sitem->getSelection();

    annotation->removeSelection(selection);
    selectionList->removeItemWidget(item2);
    delete selection;
    delete item2;
    redrawCurrentSelections();
}

bool AnnotationWidget::filterAnnotation(VisionAnnotation* annotation){
    if(!greenBox->isChecked() && annotation->getColor() == c_FIELD_GREEN)
        return true;
    if(!orangeBox->isChecked() && annotation->getColor() == c_ORANGE)
        return true;
    if(!yellowBox->isChecked() && annotation->getColor() == c_YELLOW)
        return true;
    if(!whiteBox->isChecked() && annotation->getColor() == c_WHITE)
        return true;
    if(!robotWhiteBox->isChecked() && annotation->getColor() == c_ROBOT_WHITE)
        return true;
    if(!pinkBox->isChecked() && annotation->getColor() == c_PINK)
        return true;
    if(!blueBox->isChecked() && annotation->getColor() == c_BLUE)
        return true;
    return false;
}

void AnnotationWidget::redrawCurrentSelections() {
    handleNewLogFrame(currentFrame_);
}

void AnnotationWidget::saveToFile() {
    AnnotationGroup group(getAnnotations());
    group.save(log_);
}

void AnnotationWidget::clearMovements() {
    if(selectedAnnotation_) {
        selectedAnnotation_->clearCenterPoints();
        redrawCurrentSelections();
    }
}

void AnnotationWidget::toggleDefiningMovements() {
    definingMovements_ = !definingMovements_;
    if(definingMovements_)
        defineMovementsButton->setText("Stop");
    else
        defineMovementsButton->setText("Move");
}

void AnnotationWidget::handleClick(int x, int y, int button) {
    button = button; // kill warning
    if(!definingMovements_)
        return;
    if(selectedAnnotation_) {
        selectedAnnotation_->setCenterPoint(currentFrame_, x, y);
        redrawCurrentSelections();
    }
}
