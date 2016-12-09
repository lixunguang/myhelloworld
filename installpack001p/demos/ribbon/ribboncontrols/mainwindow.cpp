#include <QFileDialog>
#include <QApplication>
#include <QDesktopWidget>
#include <QPrinter>
#include <QPrintDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QLineEdit>
#include <QRadioButton>
#include <QScrollBar>
#include <QFontComboBox>
#include <QAction>
#include <QMenu>
#include <QLabel>
#include <QSpinBox>
#include <QMessageBox>
#include <QButtonGroup>
#include <QPushButton>
#include <QSettings>
#include <QPainter>

#include <QtnOfficeDefines.h>
#include <QtnRibbonQuickAccessBar.h>

#include "aboutdialog.h"
#include "mainwindow.h"

#define Prop_ContextColor   "ContextColor"


/* CentralWidget */
class WorkspaceWidget : public QWidget
{
public:
    WorkspaceWidget() 
    {
        setAttribute(Qt::WA_MouseTracking);
        setAutoFillBackground(true);
        QPalette pal = palette();
        pal.setColor(QPalette::Background, QColor(255, 255, 255));
        setPalette(pal);
    }
protected:
    virtual void mousePressEvent(QMouseEvent* event)
    {
        Q_UNUSED(event);
        setFocus();
    }
    virtual void paintEvent(QPaintEvent* event)
    {
        Q_UNUSED(event);
        QPainter p(this);
        QStyleOption opt;
        opt.init(this);
        if (event->rect().intersects(opt.rect))
            style()->drawPrimitive((QStyle::PrimitiveElement)CommonStyle::PE_Workspace, &opt, &p, this);
    }
};

/* MainWindow */
MainWindow::MainWindow(QWidget* parent)
    : RibbonWindow(parent)
{
    m_valueProgress = 0;
    m_progressBar  = Q_NULL;
    m_progressBar1 = Q_NULL;
    m_progressBar2 = Q_NULL;
    m_pageContext = Q_NULL;

    setWindowTitle(tr("Qtitan Ribbon Controls Sample"));

    setCentralWidget(new WorkspaceWidget);

    createMenuFile();
    createQuickAccessBar();
    createRibbon();
    createStatusBar();

    m_timerProgressBar.setInterval(200);
    connect(&m_timerProgressBar, SIGNAL(timeout()), this, SLOT(animateProgressBar()));

    move(QPoint(200, 200));
    QRect geom = QApplication::desktop()->availableGeometry();
    resize(2 * geom.width() / 3, 2 * geom.height() / 3);

    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    settings.setValue("recentFileList", files);
    emit updateRecentFileActions(files);
}

MainWindow::~MainWindow()
{
}

void MainWindow::createMenuFile()
{
    QIcon iconLogo;
    iconLogo.addPixmap(QPixmap(":/shared/res/qtitan.png"));
    iconLogo.addPixmap(QPixmap(":/shared/res/qtitanlogo32x32.png"));
    if (QAction* actionFile = ribbonBar()->addSystemButton(iconLogo, tr("&File"))) 
    {
        actionFile->setToolTip(tr("Click here to see everything<br />you can do with your<br />document"));

        if (Qtitan::RibbonSystemPopupBar* popupBar = qobject_cast<Qtitan::RibbonSystemPopupBar*>(actionFile->menu()))
        {
            QAction* newFile = popupBar->addAction(QIcon(":/res/new.png"), tr("&New"));
            newFile->setShortcut(tr("Ctrl+N"));
            newFile->setStatusTip(tr("Create a new document"));
            newFile->setToolTip(tr("New"));
            newFile->setEnabled(false);

            QAction* openFile = popupBar->addAction(QIcon(":/res/open.png"), tr("&Open..."));
            openFile->setShortcut(tr("Ctrl+O"));
            openFile->setToolTip(tr("Open"));
            openFile->setStatusTip(tr("Open an existing document"));
            connect(openFile, SIGNAL(triggered()), this, SLOT(open()));

            QAction* saveFile = popupBar->addAction(QIcon(":/res/save.png"), tr("&Save"));
            saveFile->setShortcut(tr("Ctrl+S"));
            saveFile->setToolTip(tr("Save"));
            saveFile->setStatusTip(tr("Save the active document"));
            connect(saveFile, SIGNAL(triggered()), this, SLOT(save()));

            QAction* saveAsFile = popupBar->addAction(tr("Save &As..."));
            saveAsFile->setToolTip(tr("Save As"));
            saveAsFile->setStatusTip(tr("Save the active document with a new name"));
            connect(saveAsFile, SIGNAL(triggered()), this, SLOT(save()));

    #ifndef QT_NO_PRINTER
            popupBar->addSeparator();

            QAction* printFile = popupBar->addAction(QIcon(":/res/print.png"), tr("&Print"));
            printFile->setShortcut(tr("Ctrl+P"));
            printFile->setToolTip(tr("Print"));
            printFile->setStatusTip(tr("Print the active document"));
            connect(printFile, SIGNAL(triggered()), this, SLOT(print()));

            QAction* printPreviewFile = popupBar->addAction(QIcon(":/res/printPreview.png"), tr("Print Pre&view"));
            printPreviewFile->setToolTip(tr("Print Preview"));
            printPreviewFile->setStatusTip(tr("Display full pages"));

            QAction* printSetupFile = popupBar->addAction(tr("P&rint Setup..."));
            printSetupFile->setToolTip(tr("Print Setup"));
            printSetupFile->setStatusTip(tr("Change the printer and printing options"));
            connect(printSetupFile, SIGNAL(triggered()), this, SLOT(printSetup()));
            popupBar->addSeparator();
    #endif // QT_NO_PRINTER

            QIcon iconClose;
            iconClose.addPixmap(QPixmap(":/res/close.png"));
            iconClose.addPixmap(QPixmap(":/res/smallclose.png"));
            QAction* actClose = popupBar->addAction(iconClose, tr("&Close"));
            actClose->setShortcut(tr("Ctrl+C"));
            actClose->setStatusTip(tr("Exit"));
            connect(actClose, SIGNAL(triggered()), this, SLOT(close()));
            popupBar->addPopupBarAction(actClose, Qt::ToolButtonTextBesideIcon);

            QAction* option = new QAction(QPixmap(":/res/smalloption.png"), tr("Opt&ion"), this);
            popupBar->addPopupBarAction(option, Qt::ToolButtonTextBesideIcon);
            option->setEnabled(false);

            if (RibbonPageSystemRecentFileList* pageRecentFile = popupBar->addPageRecentFile(tr("Recent Documents")))
            {
                pageRecentFile->setSize(9);
                connect(pageRecentFile, SIGNAL(openRecentFile(const QString&)), 
                    this, SLOT(openRecentFile(const QString&)));
                connect(this, SIGNAL(updateRecentFileActions(const QStringList&)), 
                    pageRecentFile, SLOT(updateRecentFileActions(const QStringList&)));
            }
        }
    }
}

void MainWindow::createQuickAccessBar()
{
    if (Qtitan::RibbonQuickAccessBar* quickAccessBar = ribbonBar()->quickAccessBar())
    {
        QAction* action = quickAccessBar->actionCustomizeButton();
        action->setToolTip(tr("Customize Quick Access Bar"));

        QAction* smallButton = quickAccessBar->addAction(QIcon(":/res/smallNew.png"), tr("New"));
        smallButton->setToolTip(tr("Create a new document"));
        connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));
        quickAccessBar->setActionVisible(smallButton, false);

        smallButton = quickAccessBar->addAction(QIcon(":/res/smallOpen.png"), tr("Open"));
        smallButton->setToolTip(tr("Open an existing document"));
        connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));
        quickAccessBar->setActionVisible(smallButton, false);

        smallButton = quickAccessBar->addAction(QIcon(":/res/smallSave.png"), tr("Save"));
        smallButton->setToolTip(tr("Save the active document"));
        connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

        smallButton = quickAccessBar->addAction(QIcon(":/res/smallUndo.png"), tr("&Undo"));
        smallButton->setShortcut(QKeySequence::Undo);
        smallButton->setEnabled(false);

        smallButton = quickAccessBar->addAction(QIcon(":/res/smallRedo.png"), tr("&Redo"));
        smallButton->setShortcut(QKeySequence::Redo);
        smallButton->setEnabled(false);

        ribbonBar()->showQuickAccess(true);
    }
}

void MainWindow::createRibbon()
{
    if (Qtitan::RibbonPage* pageButtons = ribbonBar()->addPage(tr("&Buttons")))
    {
        QMenu* splitPopup = new QMenu(ribbonBar());
        connect(splitPopup->addAction(tr("Option 1")), SIGNAL(triggered()), this, SLOT(pressButton()));
        connect(splitPopup->addAction(tr("Option 2")), SIGNAL(triggered()), this, SLOT(pressButton()));
        connect(splitPopup->addAction(tr("Option 3")), SIGNAL(triggered()), this, SLOT(pressButton()));

        QIcon iconSplit;
        iconSplit.addPixmap(QPixmap(":/res/largeSplitButton.png"));
        iconSplit.addPixmap(QPixmap(":/res/smallSplitButton.png"));

        QIcon iconButtton;
        iconButtton.addPixmap(QPixmap(":/res/largeButton.png"));
        iconButtton.addPixmap(QPixmap(":/res/smallButton.png"));

        QIcon iconPopupButtton;
        iconPopupButtton.addPixmap(QPixmap(":/res/largePopupButton.png"));
        iconPopupButtton.addPixmap(QPixmap(":/res/smallPopupButton.png"));

        // Large Buttons
        Qtitan::RibbonGroup* groupClipboard = pageButtons->addGroup(tr("&Large Buttons"));
        if (groupClipboard) 
        {
            groupClipboard->setOptionButtonVisible();

            QAction* act = groupClipboard->optionButtonAction();
            act->setToolTip(tr("Large Buttons"));
            connect(act, SIGNAL(triggered()), this, SLOT(pressButton()));

            QAction* action = groupClipboard->addAction(iconButtton, tr("&Button"), Qt::ToolButtonTextUnderIcon);
            connect(action, SIGNAL(triggered()), this, SLOT(pressButton()));

            action = groupClipboard->addAction(QIcon(":/res/largeToggleButton.png"), 
                tr("Toggle Button"), Qt::ToolButtonTextUnderIcon);
            action->setCheckable(true);
            action->setChecked(true);

            QMenu* menuPopup = groupClipboard->addMenu(iconPopupButtton, tr("Popup Button"), Qt::ToolButtonTextUnderIcon);
            connect(menuPopup->addAction(tr("Option 1")), SIGNAL(triggered()), this, SLOT(pressButton()));
            connect(menuPopup->addAction(tr("Option 2")), SIGNAL(triggered()), this, SLOT(pressButton()));
            connect(menuPopup->addAction(tr("Option 3")), SIGNAL(triggered()), this, SLOT(pressButton()));

            action = groupClipboard->addAction(iconSplit, 
                tr("&Split Button"), Qt::ToolButtonTextUnderIcon, splitPopup);
            connect(action, SIGNAL(triggered()), this, SLOT(pressButton()));
        }

        // Small Buttons
        Qtitan::RibbonGroup* groupSmallButtons = pageButtons->addGroup(tr("Small Buttons"));
        if (groupSmallButtons)
        {
            groupSmallButtons->setOptionButtonVisible();
            QAction* act = groupSmallButtons->optionButtonAction();
            act->setText(tr("S&mall"));
            act->setToolTip(tr("Small Buttons"));
            connect(act, SIGNAL(triggered()), this, SLOT(pressButton()));

            QAction* smallButton = groupSmallButtons->addAction(iconButtton, tr("B&utton"), Qt::ToolButtonTextBesideIcon);
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

            QMenu* menuPopup = groupSmallButtons->addMenu(iconPopupButtton, tr("Popup Button"), Qt::ToolButtonTextBesideIcon);
            menuPopup->addAction(tr("Option &1"));
            menuPopup->addAction(tr("Option &2"));
            menuPopup->addAction(tr("Option &3"));

            smallButton = groupSmallButtons->addAction(iconSplit, tr("Split Button"), Qt::ToolButtonTextBesideIcon, splitPopup);
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

            smallButton = groupSmallButtons->addAction(iconButtton, tr("B&utton"), Qt::ToolButtonIconOnly);
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

            menuPopup = groupSmallButtons->addMenu(iconPopupButtton, tr("Popup Button"), Qt::ToolButtonIconOnly);
            menuPopup->addAction(tr("Option &1"));
            menuPopup->addAction(tr("Option &2"));
            menuPopup->addAction(tr("Option &3"));

            smallButton = groupSmallButtons->addAction(iconSplit, tr("Split Button"), Qt::ToolButtonIconOnly, splitPopup);
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));
        }

        // CheckBoxes and Radio Buttons
        Qtitan::RibbonGroup* groupCheckboxButtons = pageButtons->addGroup(tr("Checkboxes && Radio Buttons"));
        if (groupCheckboxButtons) 
        {
            groupCheckboxButtons->setOptionButtonVisible();
            QAction* act = groupCheckboxButtons->optionButtonAction();
            act->setToolTip(tr("Checkboxes && Radio Buttons"));
            connect(act, SIGNAL(triggered()), this, SLOT(pressButton()));

            groupCheckboxButtons->setControlsCentering(true);
            QCheckBox* check1 = new QCheckBox(tr("CheckBox 1"));
            check1->setToolTip(tr("CheckBox 1"));
            check1->setCheckState(Qt::Checked);
            groupCheckboxButtons->addWidget(check1);

            QCheckBox* check2 = new QCheckBox(tr("CheckBox 2"));
            check2->setToolTip(tr("CheckBox 2"));
            check2->setCheckState(Qt::Unchecked);
            groupCheckboxButtons->addWidget(check2);

            QCheckBox* check3 = new QCheckBox(tr("CheckBox 3"));
            check3->setToolTip(tr("CheckBox 3"));
            check3->setCheckState(Qt::PartiallyChecked);
            groupCheckboxButtons->addWidget(check3);

            groupCheckboxButtons->addSeparator();

            QRadioButton* radio1 = new QRadioButton(tr("Radio Button 1"));
            radio1->setToolTip(tr("Radio Button 1"));
            radio1->setChecked(true);
            groupCheckboxButtons->addWidget(radio1);

            QRadioButton* radio2 = new QRadioButton(tr("Radio Button 2"));
            radio2->setToolTip(tr("Radio Button 2"));
            groupCheckboxButtons->addWidget(radio2);

            QRadioButton* radio3 = new QRadioButton(tr("Radio Button 3"));
            radio3->setToolTip(tr("Radio Button 3"));
            groupCheckboxButtons->addWidget(radio3);
        }

        // Groups
        Qtitan::RibbonGroup* groupGroups = pageButtons->addGroup(tr("Groups"));
        if (groupGroups) 
        {
            groupGroups->setControlsGrouping(true);
            groupGroups->setControlsCentering(true);

            QAction* smallButton = groupGroups->addAction(QIcon(":/res/smallNew.png"), 
                tr("New"), Qt::ToolButtonIconOnly);
            smallButton->setToolTip(tr("Create a new document"));
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

            smallButton = groupGroups->addAction(QIcon(":/res/smallOpen.png"), 
                tr("Open"), Qt::ToolButtonIconOnly);
            smallButton->setToolTip(tr("Open an existing document"));
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

            smallButton = groupGroups->addAction(QIcon(":/res/smallSave.png"), 
                tr("Save"), Qt::ToolButtonIconOnly);
            smallButton->setToolTip(tr("Save the active document"));
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

            smallButton = groupGroups->addAction(QIcon(":/res/smallCut.png"), 
                tr("Cut"), Qt::ToolButtonIconOnly);
            smallButton->setToolTip(tr("Cut the selection and put it on the Clipboard"));
            smallButton->setEnabled(false);
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

            smallButton = groupGroups->addAction(QIcon(":/res/smallCopy.png"), 
                tr("Copy"), Qt::ToolButtonIconOnly);
            smallButton->setToolTip(tr("Copy the selection and put it on the Clipboard"));
            smallButton->setEnabled(false);
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

            smallButton = groupGroups->addAction(QIcon(":/res/smallClipboard.png"), 
                tr("Paste"), Qt::ToolButtonIconOnly);
            smallButton->setToolTip(tr("Insert Clipboard contents"));
            smallButton->setEnabled(false);
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

            groupGroups->addSeparator();

            smallButton = groupGroups->addAction(QIcon(":/res/smallUndo.png"), 
                tr("Undo"), Qt::ToolButtonIconOnly);
            smallButton->setToolTip(tr("Undo the last action"));
            smallButton->setEnabled(false);
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

            smallButton = groupGroups->addAction(QIcon(":/res/smallRedo.png"), 
                tr("Redo"), Qt::ToolButtonIconOnly);
            smallButton->setToolTip(tr("Redo the previously undone action"));
            smallButton->setEnabled(false);
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

            groupGroups->addSeparator();

            smallButton = groupGroups->addAction(QIcon(":/res/smallPrint.png"), 
                tr("Print"), Qt::ToolButtonIconOnly);
            smallButton->setToolTip(tr("Print the active document"));
            connect(smallButton, SIGNAL(triggered()), this, SLOT(pressButton()));

            smallButton = groupGroups->addAction(QIcon(":/shared/res/about.png"), 
                tr("About"), Qt::ToolButtonIconOnly);
            smallButton->setToolTip(tr("Display program information, version number and copyright"));
            connect(smallButton, SIGNAL(triggered()), this, SLOT(about()));
        }
    }
    
    if (Qtitan::RibbonPage* pageEditors = ribbonBar()->addPage(tr("&Editors")))
    {
        if (Qtitan::RibbonGroup* groupStandard = pageEditors->addGroup(tr("Standard"))) 
        {
            QLineEdit* controlEdit = new QLineEdit(); 
            controlEdit->setToolTip(tr("Edit"));
            controlEdit->setText(tr("Edit"));
            controlEdit->setMinimumWidth(150);
            controlEdit->setMaximumWidth(150);
            QAction* act = groupStandard->addWidget(controlEdit);
            act->setText(tr("Edit"));

            QComboBox* controlComboBox = new QComboBox();
            controlComboBox->setToolTip(tr("Combo"));
            controlComboBox->setMinimumWidth(150);
            controlComboBox->setMaximumWidth(150);
            controlComboBox->setEditable(true);
            controlComboBox->addItem(tr("ComboBox"));
            controlComboBox->addItem(tr("Item 1"));
            controlComboBox->addItem(tr("Item 2"));
            controlComboBox->addItem(tr("Item 3"));
            act = groupStandard->addWidget(controlComboBox);
            act->setText(tr("ComboBox"));

            QComboBox* comboBox = new QComboBox();
            comboBox->setToolTip(tr("Combo"));
            comboBox->setMinimumWidth(150);
            comboBox->setMaximumWidth(150);
            comboBox->addItem(tr("ComboBox"));
            comboBox->addItem(tr("Item 1"));
            comboBox->addItem(tr("Item 2"));
            act = groupStandard->addWidget(comboBox);
            act->setText(tr("ComboBox"));
        }

        if (Qtitan::RibbonGroup* groupExtendedEditors = pageEditors->addGroup(tr("Extended"))) 
        {
            QSpinBox* spinBox = new QSpinBox;
            spinBox->setToolTip(tr("Edit:"));
            spinBox->setMinimumWidth(150);
            groupExtendedEditors->addWidget(QIcon(":/res/smallEditor.png"), tr("Edit:"), true, spinBox);

            QComboBox* comboBox = new QComboBox(); 
            comboBox->setToolTip(tr("Combo:"));
            comboBox->setMinimumWidth(150);
            comboBox->setEditable(true);
            comboBox->addItem(tr("ComboBox"));
            comboBox->addItem(tr("Item 1"));
            comboBox->addItem(tr("Item 2"));
            groupExtendedEditors->addWidget(QIcon(":/res/smallComboBox.png"), tr("Combo:"), true, comboBox);
            comboBox = new QComboBox(); 
            comboBox->setToolTip(tr("Combo:"));
            comboBox->setMinimumWidth(150);
            comboBox->setEditable(true);
            comboBox->addItem(tr("ComboBox"));
            comboBox->addItem(tr("Item 1"));
            comboBox->addItem(tr("Item 2"));
            groupExtendedEditors->addWidget(QIcon(":/res/smallComboBox.png"), tr("Combo:"), true, comboBox);
        }
        if (Qtitan::RibbonGroup* groupGallery = pageEditors->addGroup(tr("Gallery")))
        {
            QFontComboBox* comboFont = new QFontComboBox(groupGallery);
            comboFont->setToolTip(tr("Font:"));
            comboFont->setFontFilters(QFontComboBox::MonospacedFonts);
            comboFont->setEditText(tr(""));
            comboFont->setMinimumWidth(150);
            groupGallery->addWidget(QIcon(), tr("Font:"), comboFont);
        }
    }

    if (Qtitan::RibbonPage* pageMisc = ribbonBar()->addPage(tr("&Misc")))
    {
        Qtitan::RibbonGroup* groupLable = pageMisc->addGroup(tr("Label"));
        groupLable->addWidget(new QLabel(tr("Label Text 1")));
        groupLable->addWidget(new QLabel(tr("Label Text 2")));
        groupLable->addWidget(new QLabel(tr("Label Text 3")));

        Qtitan::RibbonGroup* groupBitmap = pageMisc->addGroup(tr("Bitmap"));
        groupBitmap->setControlsCentering(true);
        QLabel* pLabel = new QLabel(tr("Label Text 1"));
        pLabel->setPixmap(QPixmap(":/res/Windows.png"));
        groupBitmap->addWidget(pLabel);

        Qtitan::RibbonGroup* groupBitmapLinux = pageMisc->addGroup(tr("Bitmap"));
        groupBitmapLinux->setControlsCentering(true);
        pLabel = new QLabel(tr("Label Text 2"));
        pLabel->setPixmap(QPixmap(":/res/linux.png"));
        groupBitmapLinux->addWidget(pLabel);

        Qtitan::RibbonGroup* groupProgressBar = pageMisc->addGroup(tr("ProgressBar"));
        groupProgressBar->setControlsCentering(true);
        m_progressBar = new QProgressBar();
        m_progressBar->setAlignment(Qt::AlignHCenter);
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(40);
        m_progressBar->setMinimumWidth(100);
        m_progressBar->setMaximumHeight(22);
        groupProgressBar->addWidget(m_progressBar);

        m_progressBar1 = new QProgressBar();
        m_progressBar1->setTextVisible(false);
        m_progressBar1->setRange(0, 100);
        m_progressBar1->setValue(40);
        m_progressBar1->setMinimumWidth(100);
        m_progressBar1->setMaximumHeight(22);
        groupProgressBar->addWidget(m_progressBar1);

        QCheckBox* checkAnimationBar = new QCheckBox(tr("Animation bar"));
        connect(checkAnimationBar, SIGNAL(stateChanged(int)), this, SLOT(startProgressBar(int)));
        checkAnimationBar->setCheckState(Qt::Unchecked);
        groupProgressBar->addWidget(checkAnimationBar);

        Qtitan::RibbonGroup* groupScrollBar = pageMisc->addGroup(tr("ScrollBar"));
        groupScrollBar->setControlsCentering(true);

        QScrollBar* scrollBar = new QScrollBar(Qt::Horizontal, Q_NULL);
        scrollBar->setMinimumWidth(100);
        scrollBar->setMaximumHeight(18);
        scrollBar->setMinimum(0);
        scrollBar->setMaximum(20);
        groupScrollBar->addWidget(scrollBar);

        Qtitan::RibbonGroup* groupSlider = pageMisc->addGroup(tr("Slider"));
        groupSlider->setControlsCentering(true);

        Qtitan::RibbonSliderPane* sliderPane = new Qtitan::RibbonSliderPane();
        sliderPane->setScrollButtons(true);
        sliderPane->setMaximumWidth(100);
        sliderPane->setMaximumHeight(18);
        sliderPane->setRange(0, 100);
        sliderPane->setSingleStep(10);
        sliderPane->setSliderPosition(50);
        groupSlider->addWidget(sliderPane);

        QSlider* slider = new QSlider(Qt::Horizontal);
        slider->setMinimumWidth(100);
        slider->setMaximumHeight(20);
        slider->setRange(0, 100);
        slider->setTickPosition(QSlider::TicksLeft);
        slider->setTickInterval(20);
        slider->setSingleStep(10);
        slider->setSliderPosition(50);
        groupSlider->addWidget(slider);

        connect(sliderPane, SIGNAL(valueChanged(int)), slider, SLOT(setValue(int)));
        connect(slider, SIGNAL(valueChanged(int)), sliderPane, SLOT(setValue(int)));

        slider = new QSlider(Qt::Horizontal);
        slider->setMinimumWidth(100);
        slider->setMaximumHeight(20);
        slider->setTickPosition(QSlider::TicksRight);
        slider->setTickInterval(20);
        groupSlider->addWidget(slider);

        groupSlider->addSeparator();

        slider = new QSlider(Qt::Vertical);
        slider->setMaximumHeight(60);
        slider->setTickPosition(QSlider::TicksLeft);
        slider->setTickInterval(20);
        groupSlider->addWidget(slider);

        slider = new QSlider(Qt::Vertical);
        slider->setMaximumHeight(60);
        slider->setTickPosition(QSlider::TicksRight);
        slider->setTickInterval(20);
        groupSlider->addWidget(slider);

        groupSlider->addSeparator();

        slider = new QSlider(Qt::Vertical);
        slider->setMaximumHeight(60);
        groupSlider->addWidget(slider);

    }

    if (Qtitan::RibbonPage* pageContext = ribbonBar()->addPage(tr("&Context Tab")))
    {
        m_pageContext = pageContext;
        m_pageContext->setContextColor(RibbonPage::ContextColorRed);

        Qtitan::RibbonGroup* groupContextColor = m_pageContext->addGroup(tr("Context Color"));

        QButtonGroup* groupColorButton = new QButtonGroup(this);
        connect(groupColorButton, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(buttonColorClicked(QAbstractButton*)));

        QRadioButton* radioGreen = new QRadioButton(tr("Green"));
        radioGreen->setToolTip(tr("Color Green"));
        radioGreen->setProperty(Prop_ContextColor, RibbonPage::ContextColorGreen);
        groupColorButton->addButton(radioGreen);
        groupContextColor->addWidget(radioGreen);

        QRadioButton* radioBlue = new QRadioButton(tr("Blue"));
        radioBlue->setToolTip(tr("Color Blue"));
        radioBlue->setProperty(Prop_ContextColor, RibbonPage::ContextColorBlue);
        groupColorButton->addButton(radioBlue);
        groupContextColor->addWidget(radioBlue);

        QRadioButton* radioRed = new QRadioButton(tr("Red"));
        radioRed->setToolTip(tr("Color Red"));
        radioGreen->setChecked(true);
        radioRed->setProperty(Prop_ContextColor, RibbonPage::ContextColorRed);
        groupColorButton->addButton(radioRed);
        groupContextColor->addWidget(radioRed);

        QRadioButton* radioYellow = new QRadioButton(tr("Yellow"));
        radioYellow->setToolTip(tr("Color Yellow"));
        radioYellow->setProperty(Prop_ContextColor, RibbonPage::ContextColorYellow);
        groupColorButton->addButton(radioYellow);
        groupContextColor->addWidget(radioYellow);

        QRadioButton* radioCyan = new QRadioButton(tr("Cyan"));
        radioCyan->setToolTip(tr("Color Cyan"));
        radioCyan->setProperty(Prop_ContextColor, RibbonPage::ContextColorCyan);
        groupColorButton->addButton(radioCyan);
        groupContextColor->addWidget(radioCyan);

        QRadioButton* radioPurple = new QRadioButton(tr("Purple"));
        radioPurple->setToolTip(tr("Color Purple"));
        radioPurple->setProperty(Prop_ContextColor, RibbonPage::ContextColorPurple);
        groupColorButton->addButton(radioPurple);
        groupContextColor->addWidget(radioPurple);

        QRadioButton* radioOrange = new QRadioButton(tr("Orange"));
        radioOrange->setToolTip(tr("Color Orange"));
        radioOrange->setProperty(Prop_ContextColor, RibbonPage::ContextColorOrange);
        groupColorButton->addButton(radioOrange);
        groupContextColor->addWidget(radioOrange);
    }
//    ribbonBar()->setTitleBarVisible(false);
    ribbonBar()->setFrameThemeEnabled();
//    ribbonBar()->setLogoPixmap(Qt::AlignRight, QPixmap(":/res/qtitan.png"));
//    ribbonBar()->setLogoPixmap(Qt::AlignLeft, QPixmap(":/res/qtitan.png"));
}

void MainWindow::createStatusBar()
{
    Qtitan::RibbonSliderPane* sliderPane = new Qtitan::RibbonSliderPane();
    sliderPane->setScrollButtons(true);
    sliderPane->setRange(0, 100); // Range
    sliderPane->setMaximumWidth(130);
    sliderPane->setSingleStep(10);
    sliderPane->setSliderPosition(50);
    statusBar()->addPermanentWidget(sliderPane);
    statusBar();
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open"));
    if (!fileName.isEmpty()) 
    {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File loaded"), 2000);
    }
}

bool MainWindow::save()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),tr("Document"), QLatin1Char('*') + QLatin1String(".txt"));
    if (!fileName.isEmpty())
    {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File saved"), 2000);
        return true;
    }
    return false;
}

void MainWindow::openRecentFile(const QString& file)
{
    QMessageBox messageBox(QMessageBox::Information, windowTitle(), 
        QLatin1String(""), QMessageBox::Ok, this);
    messageBox.setInformativeText(QLatin1String("Open Recent File.") + QLatin1String(" ") + file);
    messageBox.exec();
}

#ifndef QT_NO_PRINTER
void MainWindow::print()
{
    QPrinter printer;
    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted)
    {
    }
}

void MainWindow::printSetup()
{
}
#endif

void MainWindow::pressButton()
{
    QMessageBox messageBox(QMessageBox::Information, windowTitle(), 
        QLatin1String(""), QMessageBox::Ok, this);
    messageBox.setInformativeText(QLatin1String("Press button."));
    messageBox.exec();
}

void MainWindow::startProgressBar(int state)
{
    if (state == Qt::Checked)
    {
        m_progressBar2 = new QProgressBar();
        m_progressBar2->setMaximumWidth(200);
        int height = statusBar()->sizeHint().height();
        m_progressBar2->setMaximumHeight(height - height/3);
        statusBar()->addPermanentWidget(m_progressBar2);

        m_valueProgress = 0;
        m_timerProgressBar.start();
    }
    else if (state == Qt::Unchecked)
    {
        m_timerProgressBar.stop();
        m_valueProgress = 40;
        m_progressBar->setValue(m_valueProgress);
        m_progressBar1->setValue(m_valueProgress);
        m_progressBar2->setValue(m_valueProgress);
        statusBar()->removeWidget(m_progressBar2);

        delete m_progressBar2;
        m_progressBar2 = Q_NULL;
    }
}

void MainWindow::animateProgressBar()
{
    m_progressBar->setValue(m_valueProgress);
    m_progressBar1->setValue(m_valueProgress);
    m_progressBar2->setValue(m_valueProgress);

    m_valueProgress += 10;    

    if (m_valueProgress == 100)
    {
        m_progressBar->setValue(m_valueProgress);
        m_progressBar1->setValue(m_valueProgress);
        m_progressBar2->setValue(m_valueProgress);
        m_valueProgress = 0;
    }
}

void MainWindow::buttonColorClicked(QAbstractButton* but)
{
    uint numColor = but->property(Prop_ContextColor).toUInt();
    m_pageContext->setContextColor((RibbonPage::ContextColor)numColor);
}

void MainWindow::setCurrentFile(const QString& fileName)
{
    QString curFile = fileName;
    setWindowFilePath(curFile);

    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);

    while (files.size() > 9)
        files.removeLast();

    settings.setValue("recentFileList", files);

    emit updateRecentFileActions(files);
}
