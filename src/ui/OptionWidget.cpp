/**************************************************************************
* Otter Browser: Web browser controlled by the user, not vice-versa.
* Copyright (C) 2013 - 2017 Michal Dutkiewicz aka Emdek <michal@emdek.pl>
* Copyright (C) 2016 - 2017 Piotr Wójcik <chocimier@tlen.pl>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
**************************************************************************/

#include "OptionWidget.h"
#include "ColorWidget.h"
#include "IconWidget.h"
#include "FilePathWidget.h"

#include <QtWidgets/QHBoxLayout>

namespace Otter
{

OptionWidget::OptionWidget(const QVariant &value, SettingsManager::OptionType type, QWidget *parent) : QWidget(parent),
	m_widget(nullptr),
	m_colorWidget(nullptr),
	m_filePathWidget(nullptr),
	m_iconWidget(nullptr),
	m_comboBox(nullptr),
	m_fontComboBox(nullptr),
	m_lineEdit(nullptr),
	m_spinBox(nullptr),
	m_resetButton(nullptr),
	m_type(type)
{
	switch (type)
	{
		case SettingsManager::BooleanType:
			m_widget = m_comboBox = new QComboBox(this);

			m_comboBox->addItem(tr("No"), QLatin1String("false"));
			m_comboBox->addItem(tr("Yes"), QLatin1String("true"));
			m_comboBox->setCurrentIndex(value.toBool() ? 1 : 0);

			connect(m_comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(markAsModified()));

			break;
		case SettingsManager::ColorType:
			m_widget = m_colorWidget = new ColorWidget(this);

			m_colorWidget->setColor(value.value<QColor>());

			connect(m_colorWidget, SIGNAL(colorChanged(QColor)), this, SLOT(markAsModified()));

			break;
		case SettingsManager::EnumerationType:
			m_widget = m_comboBox = new QComboBox(this);
			m_comboBox->addItem(value.toString(), value);
			m_comboBox->setCurrentIndex(0);

			connect(m_comboBox, SIGNAL(currentTextChanged(QString)), this, SLOT(markAsModified()));

			break;
		case SettingsManager::FontType:
			m_widget = m_fontComboBox = new QFontComboBox(this);

			m_fontComboBox->setCurrentFont(QFont(value.toString()));
			m_fontComboBox->lineEdit()->selectAll();

			connect(m_fontComboBox, SIGNAL(currentFontChanged(QFont)), this, SLOT(markAsModified()));

			break;
		case SettingsManager::IconType:
			m_widget = m_iconWidget = new IconWidget(this);

			if (value.type() == QVariant::String)
			{
				m_iconWidget->setIcon(value.toString());
			}
			else
			{
				m_iconWidget->setIcon(value.value<QIcon>());
			}

			connect(m_iconWidget, SIGNAL(iconChanged(QIcon)), this, SLOT(markAsModified()));

			break;
		case SettingsManager::IntegerType:
			m_widget = m_spinBox = new QSpinBox(this);

			m_spinBox->setMinimum(-999999999);
			m_spinBox->setMaximum(999999999);
			m_spinBox->setValue(value.toInt());
			m_spinBox->selectAll();

			connect(m_spinBox, SIGNAL(valueChanged(int)), this, SLOT(markAsModified()));

			break;
		case SettingsManager::PathType:
			m_widget = m_filePathWidget = new FilePathWidget(this);

			m_filePathWidget->setPath(value.toString());
			m_filePathWidget->setSelectFile(false);

			connect(m_filePathWidget, SIGNAL(pathChanged(QString)), this, SLOT(markAsModified()));

			break;
		default:
			m_widget = m_lineEdit = new QLineEdit(((value.type() == QVariant::StringList) ? value.toStringList().join(QLatin1String(", ")) : value.toString()), this);

			m_lineEdit->setClearButtonEnabled(true);
			m_lineEdit->selectAll();

			if (type == SettingsManager::PasswordType)
			{
				m_lineEdit->setEchoMode(QLineEdit::PasswordEchoOnEdit);
			}

			connect(m_lineEdit, SIGNAL(textChanged(QString)), this, SLOT(markAsModified()));

			break;
	}

	setAutoFillBackground(false);

	QHBoxLayout *layout(new QHBoxLayout(this));
	layout->setMargin(0);
	layout->addWidget(m_widget);

	setLayout(layout);

	m_widget->setAutoFillBackground(false);
	m_widget->setFocusPolicy(Qt::StrongFocus);
	m_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void OptionWidget::focusInEvent(QFocusEvent *event)
{
	QWidget::focusInEvent(event);

	if (m_widget)
	{
		m_widget->setFocus();
	}
}

void OptionWidget::markAsModified()
{
	if (m_resetButton)
	{
		m_resetButton->setEnabled(m_defaultValue != getValue());
	}

	emit commitData(this);
}

void OptionWidget::reset()
{
	setValue(getDefaultValue());

	if (m_resetButton)
	{
		m_resetButton->setEnabled(false);
	}
}

void OptionWidget::setDefaultValue(const QVariant &value)
{
	m_defaultValue = value;

	if (m_iconWidget)
	{
		if (value.type() == QVariant::Icon)
		{
			m_iconWidget->setDefaultIcon(value.value<QIcon>());
		}
		else
		{
			m_iconWidget->setDefaultIcon(value.toString());
		}
	}

	if (!m_resetButton)
	{
		m_resetButton = new QPushButton(tr("Defaults"), this);
		m_resetButton->setEnabled(m_defaultValue != getValue());

		layout()->addWidget(m_resetButton);

		connect(m_resetButton, SIGNAL(clicked()), this, SLOT(reset()));
	}
}

void OptionWidget::setValue(const QVariant &value)
{
	if (m_colorWidget)
	{
		m_colorWidget->setColor(value.value<QColor>());
	}
	else if (m_comboBox)
	{
		m_comboBox->setCurrentIndex(qMax(0, m_comboBox->findData(value)));
	}
	else if (m_filePathWidget)
	{
		m_filePathWidget->setPath(value.toString());
	}
	else if (m_fontComboBox)
	{
		m_fontComboBox->setCurrentText(value.toString());
	}
	else if (m_iconWidget)
	{
		if (value.type() == QVariant::String)
		{
			m_iconWidget->setIcon(value.toString());
		}
		else
		{
			m_iconWidget->setIcon(value.value<QIcon>());
		}
	}
	else if (m_lineEdit)
	{
		m_lineEdit->setText((value.type() == QVariant::StringList) ? value.toStringList().join(QLatin1String(", ")) : value.toString());
	}
	else if (m_spinBox)
	{
		m_spinBox->setValue(value.toInt());
	}
}

void OptionWidget::setChoices(const QStringList &choices)
{
	if (!m_comboBox)
	{
		return;
	}

	const QVariant value(m_comboBox->currentData());

	m_comboBox->clear();

	for (int i = 0; i < choices.count(); ++i)
	{
		if (choices.at(i).isEmpty())
		{
			m_comboBox->insertSeparator(i);
		}
		else
		{
			m_comboBox->addItem(choices.at(i), choices.at(i));
		}
	}

	setValue(value);
}

void OptionWidget::setChoices(const QVector<SettingsManager::OptionDefinition::ChoiceDefinition> &choices)
{
	if (!m_comboBox)
	{
		return;
	}

	const QVariant value(m_comboBox->currentData());
	bool hasIcons(false);

	m_comboBox->clear();

	for (int i = 0; i < choices.count(); ++i)
	{
		if (!choices.at(i).icon.isNull())
		{
			hasIcons = true;

			break;
		}
	}

	for (int i = 0; i < choices.count(); ++i)
	{
		if (choices.at(i).isValid())
		{
			m_comboBox->addItem(choices.at(i).icon, choices.at(i).getTitle(), (choices.at(i).value.isEmpty() ? QVariant() : choices.at(i).value));

			if (hasIcons && choices.at(i).icon.isNull())
			{
				m_comboBox->setItemData(i, QColor(Qt::transparent), Qt::DecorationRole);
			}
		}
		else
		{
			m_comboBox->insertSeparator(i);
		}
	}

	setValue(value);
}

void OptionWidget::setSizePolicy(QSizePolicy::Policy horizontal, QSizePolicy::Policy vertical)
{
	QWidget::setSizePolicy(horizontal, vertical);

	m_widget->setSizePolicy(horizontal, vertical);
}

void OptionWidget::setSizePolicy(QSizePolicy policy)
{
	QWidget::setSizePolicy(policy);

	m_widget->setSizePolicy(policy);
}

QVariant OptionWidget::getDefaultValue() const
{
	return m_defaultValue;
}

QVariant OptionWidget::getValue() const
{
	if (m_colorWidget)
	{
		return m_colorWidget->getColor();
	}

	if (m_comboBox)
	{
		return m_comboBox->currentData();
	}

	if (m_filePathWidget)
	{
		return m_filePathWidget->getPath();
	}

	if (m_fontComboBox)
	{
		return m_fontComboBox->currentFont().family();
	}

	if (m_iconWidget)
	{
		return m_iconWidget->getIcon();
	}

	if (m_lineEdit)
	{
		if (m_lineEdit->text().isEmpty())
		{
			return QVariant();
		}

		if (m_type == SettingsManager::ListType)
		{
			return m_lineEdit->text().split(QLatin1String(", "), QString::SkipEmptyParts);
		}

		return m_lineEdit->text();
	}

	if (m_spinBox)
	{
		return m_spinBox->value();
	}

	return QVariant();
}

}
