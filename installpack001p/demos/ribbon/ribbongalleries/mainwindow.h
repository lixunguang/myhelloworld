#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ribbonwindow.h"


namespace Qtitan
{
    class RibbonGalleryGroup;
    class RibbonGallery;
}

/* MainWindow */
class MainWindow : public RibbonWindow
{
  Q_OBJECT
public:
    MainWindow(QWidget* parent = Q_NULL);
    virtual ~MainWindow();

private:
    void createMenuFile();
    void createQuickAccessBar();
    void createRibbon();
    void createStatusBar();
    void createGalleryItems();

protected Q_SLOTS:
    void open();
    bool save();
#ifndef QT_NO_PRINTER
    void print();
    void printSetup();
#endif
    void pressButton();
    void aboutToHideUndo();
//    void pressButtonUndo();
    void itemColorPressed(RibbonGalleryItem* item);
    void itemStyleFontPressed(RibbonGalleryItem* item);
    void itemTablePressed(RibbonGalleryItem* item);
    void itemUndoPressed(RibbonGalleryItem* item);

private:
    QAction* m_actionUndo;

    Qtitan::RibbonGallery* m_widgetFontTextColor;
    Qtitan::RibbonGallery* m_widgetStyleFont;
    Qtitan::RibbonGallery* m_popupStyleFont;
    Qtitan::RibbonGallery* m_popupTableGallery;
    Qtitan::RibbonGallery* m_popupUndoGallery;

    Qtitan::RibbonGalleryGroup* m_itemsFontTextColor;
    Qtitan::RibbonGalleryGroup* m_itemsStyles;
    Qtitan::RibbonGalleryGroup* m_itemsTable;
    Qtitan::RibbonGalleryGroup* m_itemsUndo;
    Qtitan::RibbonGalleryGroup* m_itemsShapes;

    Qtitan::RibbonStyle* m_ribbonStyle;
};

#endif // MAINWINDOW_H
