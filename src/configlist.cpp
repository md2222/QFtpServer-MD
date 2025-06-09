#include "configlist.h"
#include <QDebug>
#include <QFontDialog>
#include <QFileDialog>
#include <QFontInfo>
#include <QCheckBox>
#include <QComboBox>
#include <QMessageBox>
#include <QHelpEvent>
#include <QToolTip>


ConfigList::ConfigList(QWidget *parent)
    : QTableWidget(parent)
{
    setColumnCount(3);
    setColumnWidth(0, 200);
    setColumnWidth(1, 200);
    setColumnWidth(2, 200);

    setHorizontalHeaderLabels(QStringList() << "Name" << "Value" << "Comment");

    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setWordWrap(false);

    horizontalHeader()->setMinimumSectionSize(50);
    //horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    horizontalHeader()->setStretchLastSection(true);
    //horizontalHeader()->setVisible(false);
    verticalHeader()->setDefaultSectionSize(verticalHeader()->defaultSectionSize() - 2);
    verticalHeader()->setVisible(false);

    QVariant v = parent->property("qss");
    // If no such property exists, the returned variant is invalid
    if (v.isValid())
        isQss = v.toBool();

    if (!isQss)
    {
        setStyleSheet("QTableView::item {}");

        QColor bgColor = palette().color(QPalette::Base);
        if (bgColor.isValid())
        {
            QColor gridColor;
            if (bgColor.lightness() < 100)
                gridColor = bgColor.lighter(170);
            else
                gridColor = bgColor.darker(120);

            //qDebug() << "CConfigList:    " << bgColor.name() << gridColor.name();
            setStyleSheet("QTableWidget { gridline-color: " + gridColor.name() + "; }");
        }
    }

    //setToolTipDuration(6000);

    //connect(itemDelegate(), &QAbstractItemDelegate::closeEditor, this, &CConfigList::onCloseEditor);
    connect(this->itemDelegate(), SIGNAL(closeEditor(QWidget*, QAbstractItemDelegate::EndEditHint)), this, SLOT(onCloseEditor(QWidget*, QAbstractItemDelegate::EndEditHint)));
}


bool ConfigList::viewportEvent(QEvent *ev)
{
    bool rc = QTableWidget::viewportEvent(ev);

    if (ev->type() == QEvent::ToolTip)
    {
        QHelpEvent *helpEvent = static_cast<QHelpEvent*>(ev);
        QModelIndex index = indexAt(helpEvent->pos());

        if (index.isValid())
        {
            QSize sizeHint = itemDelegate(index)->sizeHint(viewOptions(), index);
            QRect rItem(0, 0, sizeHint.width(), sizeHint.height());
            QRect rVisual = visualRect(index);
            if (rItem.width() > rVisual.width())
            {
                QToolTip::showText(helpEvent->globalPos(), itemFromIndex(index)->text(), this, rVisual, 6000);
            }
            //QToolTip::hideText();
        }
    }

    //return QTableWidget::viewportEvent(ev);
    return rc;
}


QHeaderView* ConfigList::header()
{
    return horizontalHeader();
}


void ConfigList::addParams(QVector<ParamStruct> &p)
{
    for (int i = 0; i < p.size(); i++)
        addParam(p[i]);
}


void ConfigList::addParam(ParamStruct &param)
{
    qDebug() << "ConfigList::addParam:  " << param.name << param.value;
    pars.append(param);

    int i = rowCount();
    insertRow(i);

    QTableWidgetItem *item = new QTableWidgetItem(param.label);
    setItem(i, 0, item);

    if (param.type == TypeHeader)
    {
        QFont f(item->font());
        f.setBold(true);
        item->setFont(f);

        setSpan(i, 0, 1, 3);
        return;
    }

    QWidget* widget = 0;

    if (param.type == TypeNumber)
    {
        item = new QTableWidgetItem(QString::number(param.value.toInt()));
    }
    else if (param.type == TypeFloat)
    {
        item = new QTableWidgetItem(QString::number(param.value.toFloat()));
    }
    else if (param.type == TypeBool)
    {
        item = new QTableWidgetItem("", TypeBool);

        QCheckBox *ch = new QCheckBox(this);
        ch->setStyleSheet("margin-left: 5px;");
        ch->setProperty("row", i);
        ch->setChecked(param.value.toBool());
        connect(ch, &QCheckBox::stateChanged, this, &ConfigList::onCheckBox);
        //setCellWidget(i, 1, ch);
        widget = ch;
    }
    else if (param.type == TypeList)
    {
        item = new QTableWidgetItem("", TypeList);

        if (pars[i].list.count() > 0)
        {
            QComboBox* cb = new QComboBox(this);
            cb->setProperty("row", i);
            cb->setEditable(true);
            foreach (QString str, pars[i].list)
                cb->addItem(str);
            cb->setEditText(param.value.toString());
            connect(cb, &QComboBox::currentTextChanged, this, &ConfigList::onComboBox);
            widget = cb;
        }
    }
    else if (param.type == TypeFont)
    {
        //QFont font = param.value.value<QFont>();
        QFont font; font.fromString(param.value.toString());
        QString s;
        if (font.fromString(param.value.toString()))
            s = font.family() + ", " + QString::number(font.pointSizeF()) + ", " + font.styleName();
        else
            s = "[Invalid font]";
        qDebug() << "addParam:  " << s;
        item = new QTableWidgetItem(s, TypeFont);
        //item->setData(Qt::FontRole, font);
    }
    else if (param.type == TypeFile)
    {
        item = new QTableWidgetItem(param.value.toString(), TypeFile);
    }
    else if (param.type == TypeDir)
    {
        item = new QTableWidgetItem(param.value.toString(), TypeDir);
    }
    else
    {
        item = new QTableWidgetItem(param.value.toString());
    }

    setItem(i, 1, item);
    if (widget)
        setCellWidget(i, 1, widget);

    item = new QTableWidgetItem(param.comment);
    setItem(i, 2, item);
}


ParamStruct* ConfigList::param(const QString& name)
{
    ParamStruct* p = 0;

    for (int i = 0; i < pars.size(); i++)
    {
        if (pars[i].type == ConfigList::TypeHeader)  continue;
        if (pars[i].name == name)
            p = &pars[i];
    }

    return p;
}

void ConfigList::editCurrentRow(int col)
{
    QTableWidgetItem* item = currentItem();

    if (item->column() == col)
    {
        int i = item->row();

        if (item->type() == TypeFont)
        {
            qDebug() << "editCurrentRow:  TypeFont";

            bool ok;
            QFont font = QFontDialog::getFont(&ok, pars[i].value.value<QFont>(), this, "Default text font", QFontDialog::DontUseNativeDialog);
            if (ok)
            {
                pars[i].value = font.toString();
                pars[i].isEdited = true;
                //QFontInfo fi(font); // lie
                QString s = font.family() + ", " + QString::number(font.pointSizeF()) + ", " + font.styleName();
                item->setText(s);
            }
        }
        else if (item->type() == TypeFile)
        {
            qDebug() << "editCurrentRow:  TypeFile";

            QFileDialog dlg(this, tr("Select file"));
            if (!isUseNatFileDlg)
                dlg.setOption(QFileDialog::DontUseNativeDialog);
            //dlg.setFilter(QDir::Hidden | QDir::NoDotAndDotDot);
            dlg.setNameFilters(QStringList()
                << "All files (*)"
            );
            dlg.setAcceptMode(QFileDialog::AcceptOpen);
            dlg.setFileMode(QFileDialog::ExistingFile);
            dlg.setViewMode(QFileDialog::Detail);
            dlg.resize(fileDialogSize);

            if (dlg.exec() != QDialog::Accepted)
                return;

            QString fileName = dlg.selectedFiles().first();
            fileDialogSize = dlg.size();

            pars[i].value = fileName;
            pars[i].isEdited = true;
            item->setText(fileName);
        }
        else if (item->type() == TypeDir)
        {
            qDebug() << "editCurrentRow:  TypeFile";

            QFileDialog dlg(this, tr("Select directory"));
            if (!isUseNatFileDlg)
                dlg.setOption(QFileDialog::DontUseNativeDialog);
            //dlg.setFilter(QDir::Hidden | QDir::NoDotAndDotDot);
            dlg.setFileMode(QFileDialog::DirectoryOnly);
            dlg.resize(fileDialogSize);

            if (dlg.exec() != QDialog::Accepted)
                return;

            QString dirPath = dlg.selectedFiles().first();
            fileDialogSize = dlg.size();

            pars[i].value = dirPath;
            pars[i].isEdited = true;
            item->setText(dirPath);
        }
        else if (item->type() == TypeBool)
        {
            ;
        }
        else
        {
            item->setFlags(item->flags () | Qt::ItemIsEditable);
            editItem(item);
            item->setFlags(item->flags () & ~Qt::ItemIsEditable);
            if (pars[i].isValid)
                editStr = item->text();
        }
    }
}


void ConfigList::onCheckBox(int state)
{
    QCheckBox* ch = qobject_cast<QCheckBox*>(sender());
    int i = ch->property("row").toInt();
    qDebug() << "onCheckBox:  " << i << ch->isChecked();
    pars[i].value = ch->isChecked();
    pars[i].isEdited = true;
}


void ConfigList::onComboBox(QString text)
{
    QComboBox* cb = qobject_cast<QComboBox*>(sender());
    int i = cb->property("row").toInt();
    qDebug() << "onComboBox:  " << i << cb->currentText();
    pars[i].value = cb->currentText();
    pars[i].isEdited = true;
}


void ConfigList::clear()
{
    pars.clear();
    QTableWidget::clearContents();
    setRowCount(0);
}


QVector<ParamStruct>& ConfigList::params()
{
    return pars;
}


void ConfigList::onCloseEditor(QWidget *editor, QAbstractItemDelegate::EndEditHint hint)
{
    QString text = currentItem()->text().trimmed();
    int i = currentRow();
    qDebug() << "onCloseEditor:  " << i << text;
    QString err;

    if (hint == QAbstractItemDelegate::RevertModelCache)
    {
        //restore value on Esc
        currentItem()->setText(editStr);
        editStr = "";
        pars[i].isValid = true;
        return;
    }

    if (pars[i].type == TypeNumber)
    {
        bool ok;
        int n = text.toInt(&ok);
        if (!ok)
            err = "Value must be integer.        \n" + pars[i].label;
        else
            pars[i].value = text.toInt();
    }
    else if (pars[i].type == TypeFloat)
    {
        bool ok;
        float n = text.replace(',', '.').toFloat(&ok);
        if (!ok)
            err = "Value must be float.        \n" + pars[i].label;
        else
            pars[i].value = text.toFloat();
    }
    else if (pars[i].type == TypeDir && !text.isEmpty())
    {
        QDir dir(text);
        if (!dir.exists())
            err = "Directory not exists.        \n" + pars[i].label;
        else
            pars[i].value = text;
    }
    else
        pars[i].value = text;

    if (!err.isEmpty())
    {
        QMessageBox::warning(this, "Options", err, QMessageBox::Ok);
        pars[i].isValid = false;
        editCurrentRow(1);
        return;
    }

    pars[i].isValid = true;
    pars[i].isEdited = true;
}


void ConfigList::loadValues(QVector<ParamStruct> &params, QSettings& set)
{
    for (int i = 0; i < params.size(); i++)
    {
        if (params[i].type == ConfigList::TypeHeader)  continue;

        //params[i].value = set.value(params[i].name, params[i].defValue);
        QVariant value = set.value(params[i].name, params[i].defValue);
        if (params[i].type == ConfigList::TypeNumber)
            params[i].value = value.toInt();
        else if (params[i].type == ConfigList::TypeFloat)
            params[i].value = value.toFloat();
        else if (params[i].type == ConfigList::TypeBool)
            params[i].value = value.toBool();
        else
            params[i].value = value.toString();
        qDebug() << "ConfigList::loadValues:  >> " << params[i].name << params[i].value;
    }
}


//void ConfigList::saveValues(QVector<ParamStruct> &params, QSettings &set)
void ConfigList::saveValues(QSettings &set)
{
    qDebug() << "ConfigList::saveValues:  ";
    for (int i = 0; i < pars.count(); i++)
    {
        if (pars[i].type == ConfigList::TypeHeader)  continue;

        //qDebug() << "WinMain::onOptions:  << " << winConf->params()[i].name << winConf->params()[i].value;
        QVariant value = pars[i].value;

        if (value.isNull())
            set.setValue(pars[i].name, pars[i].defValue);
        else if (pars[i].isEdited)
        {
            set.setValue(pars[i].name, value);
            qDebug() << "ConfigList::saveValues:    setValue:  " << pars[i].name << pars[i].value;
        }
    }
}
