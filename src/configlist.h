#ifndef CONFIGLIST_H
#define CONFIGLIST_H

#include <QTableWidget>
#include <QTableView>
#include <QHeaderView>
#include <QAbstractItemDelegate>
#include <QSettings>


struct ParamStruct
{
    QString label;
    QString name;
    QVariant value;
    QVariant defValue;
    QString strValue;
    QStringList list;
    int type = 0;
    QString comment;
    bool isEdited = false;
    bool isValid = true;
};


class ConfigList : public QTableWidget
{
    Q_OBJECT

public:
    ConfigList(QWidget *parent);
    bool isEdited = false;
    bool isUseNatFileDlg = true;
    bool isQss = false;
    QSize fileDialogSize = QSize(700, 600);
    void addParam(ParamStruct &param);
    void addParams(QVector<ParamStruct> &p);
    ParamStruct* param(const QString& name);
    void editCurrentRow(int col);
    void clear();
    QVector<ParamStruct>& params();
    QHeaderView* header();
    static void loadValues(QVector<ParamStruct> &params, QSettings& set);
    void saveValues(QSettings& set);

    enum {
        TypeHeader = 0,
        TypeString = 1,
        TypeNumber = 3,
        TypeFloat = 4,
        TypeBool = 7,
        TypeList = 9,
        TypeFont = 10,
        TypeFile = 11,
        TypeDir = 12
    };

private slots:
    void onCheckBox(int state);
    void onComboBox(QString text);
    void onCloseEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint);

private:
    QString editStr;
    QVector<ParamStruct> pars;
    bool viewportEvent(QEvent *ev);
};


#endif // CONFIGLIST_H
