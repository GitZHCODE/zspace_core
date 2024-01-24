# -*- coding: utf-8 -*-

################################################################################
## Form generated from reading UI file 'adjustFreeCameraUI.ui'
##
## Created by: Qt User Interface Compiler version 5.15.2
##
## WARNING! All changes made in this file will be lost when recompiling UI file!
################################################################################

from PySide2.QtCore import *
from PySide2.QtGui import *
from PySide2.QtWidgets import *


class Ui_AdjustFreeCamera(object):
    def setupUi(self, AdjustFreeCamera):
        if not AdjustFreeCamera.objectName():
            AdjustFreeCamera.setObjectName(u"AdjustFreeCamera")
        AdjustFreeCamera.resize(331, 140)
        self.verticalLayout = QVBoxLayout(AdjustFreeCamera)
        self.verticalLayout.setObjectName(u"verticalLayout")
        self.horizontalLayout = QHBoxLayout()
        self.horizontalLayout.setObjectName(u"horizontalLayout")
        self.verticalLayout_2 = QVBoxLayout()
        self.verticalLayout_2.setObjectName(u"verticalLayout_2")
        self.overrideNear = QCheckBox(AdjustFreeCamera)
        self.overrideNear.setObjectName(u"overrideNear")
        self.overrideNear.setFocusPolicy(Qt.NoFocus)

        self.verticalLayout_2.addWidget(self.overrideNear)

        self.overrideFar = QCheckBox(AdjustFreeCamera)
        self.overrideFar.setObjectName(u"overrideFar")
        self.overrideFar.setFocusPolicy(Qt.NoFocus)

        self.verticalLayout_2.addWidget(self.overrideFar)


        self.horizontalLayout.addLayout(self.verticalLayout_2)

        self.verticalLayout_3 = QVBoxLayout()
        self.verticalLayout_3.setObjectName(u"verticalLayout_3")
        self.nearSpinBox = QDoubleSpinBox(AdjustFreeCamera)
        self.nearSpinBox.setObjectName(u"nearSpinBox")
        sizePolicy = QSizePolicy(QSizePolicy.Preferred, QSizePolicy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(self.nearSpinBox.sizePolicy().hasHeightForWidth())
        self.nearSpinBox.setSizePolicy(sizePolicy)
        self.nearSpinBox.setDecimals(3)
        self.nearSpinBox.setMinimum(0.000000000000000)
        self.nearSpinBox.setMaximum(1000000000000.000000000000000)
        self.nearSpinBox.setSingleStep(1.000000000000000)

        self.verticalLayout_3.addWidget(self.nearSpinBox)

        self.farSpinBox = QDoubleSpinBox(AdjustFreeCamera)
        self.farSpinBox.setObjectName(u"farSpinBox")
        sizePolicy.setHeightForWidth(self.farSpinBox.sizePolicy().hasHeightForWidth())
        self.farSpinBox.setSizePolicy(sizePolicy)
        self.farSpinBox.setDecimals(3)
        self.farSpinBox.setMinimum(0.000000000000000)
        self.farSpinBox.setMaximum(1000000000000.000000000000000)
        self.farSpinBox.setSingleStep(1.000000000000000)

        self.verticalLayout_3.addWidget(self.farSpinBox)


        self.horizontalLayout.addLayout(self.verticalLayout_3)


        self.verticalLayout.addLayout(self.horizontalLayout)

        self.horizontalLayout_2 = QHBoxLayout()
        self.horizontalLayout_2.setObjectName(u"horizontalLayout_2")
        self.lockFreeCamAspect = QCheckBox(AdjustFreeCamera)
        self.lockFreeCamAspect.setObjectName(u"lockFreeCamAspect")
        self.lockFreeCamAspect.setFocusPolicy(Qt.NoFocus)

        self.horizontalLayout_2.addWidget(self.lockFreeCamAspect)

        self.freeCamAspect = QDoubleSpinBox(AdjustFreeCamera)
        self.freeCamAspect.setObjectName(u"freeCamAspect")
        self.freeCamAspect.setDecimals(3)
        self.freeCamAspect.setMinimum(0.000000000000000)
        self.freeCamAspect.setSingleStep(0.100000000000000)

        self.horizontalLayout_2.addWidget(self.freeCamAspect)


        self.verticalLayout.addLayout(self.horizontalLayout_2)

        self.horizontalLayout_3 = QHBoxLayout()
        self.horizontalLayout_3.setObjectName(u"horizontalLayout_3")
        self.freeCamFovLabel = QLabel(AdjustFreeCamera)
        self.freeCamFovLabel.setObjectName(u"freeCamFovLabel")
        self.freeCamFovLabel.setFocusPolicy(Qt.NoFocus)

        self.horizontalLayout_3.addWidget(self.freeCamFovLabel)

        self.freeCamFov = QDoubleSpinBox(AdjustFreeCamera)
        self.freeCamFov.setObjectName(u"freeCamFov")
        self.freeCamFov.setDecimals(3)
        self.freeCamFov.setMinimum(0.000000000000000)
        self.freeCamFov.setMaximum(180.000000000000000)
        self.freeCamFov.setSingleStep(1.000000000000000)

        self.horizontalLayout_3.addWidget(self.freeCamFov)


        self.verticalLayout.addLayout(self.horizontalLayout_3)


        self.retranslateUi(AdjustFreeCamera)

        QMetaObject.connectSlotsByName(AdjustFreeCamera)
    # setupUi

    def retranslateUi(self, AdjustFreeCamera):
        AdjustFreeCamera.setWindowTitle(QCoreApplication.translate("AdjustFreeCamera", u"Adjust Free Camera", None))
        AdjustFreeCamera.setProperty("comment", QCoreApplication.translate("AdjustFreeCamera", u"\n"
"     Copyright 2016 Pixar                                                                   \n"
"                                                                                            \n"
"     Licensed under the Apache License, Version 2.0 (the \"Apache License\")      \n"
"     with the following modification; you may not use this file except in                   \n"
"     compliance with the Apache License and the following modification to it:               \n"
"     Section 6. Trademarks. is deleted and replaced with:                                   \n"
"                                                                                            \n"
"     6. Trademarks. This License does not grant permission to use the trade                 \n"
"        names, trademarks, service marks, or product names of the Licensor                  \n"
"        and its affiliates, except as required to comply with Section 4(c) of               \n"
"        the License and to reproduce the content of the NOTI"
                        "CE file.                        \n"
"                                                                                            \n"
"     You may obtain a copy of the Apache License at                                         \n"
"                                                                                            \n"
"         http://www.apache.org/licenses/LICENSE-2.0                                         \n"
"                                                                                            \n"
"     Unless required by applicable law or agreed to in writing, software                    \n"
"     distributed under the Apache License with the above modification is                    \n"
"     distributed on an \"AS IS\" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY   \n"
"     KIND, either express or implied. See the Apache License for the specific               \n"
"     language governing permissions and limitations under the Apache License.               \n"
"  ", None))
        self.overrideNear.setText(QCoreApplication.translate("AdjustFreeCamera", u"Override Near", None))
        self.overrideFar.setText(QCoreApplication.translate("AdjustFreeCamera", u"Override Far", None))
        self.lockFreeCamAspect.setText(QCoreApplication.translate("AdjustFreeCamera", u"Aspect Ratio", None))
        self.freeCamFovLabel.setText(QCoreApplication.translate("AdjustFreeCamera", u"FOV (degrees)", None))
    # retranslateUi

