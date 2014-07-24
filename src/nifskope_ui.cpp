﻿/***** BEGIN LICENSE BLOCK *****

BSD License

Copyright (c) 2005-2014, NIF File Format Library and Tools
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
3. The name of the NIF File Format Library and Tools project may not be
used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

***** END LICENCE BLOCK *****/

#include "nifskope.h"
#include "version.h"
#include "options.h"

#include "ui_nifskope.h"
#include "ui/about_dialog.h"
#include "ui/settingsdialog.h"

#include "glview.h"
#include "gl/glscene.h"
#include "kfmmodel.h"
#include "nifmodel.h"
#include "nifproxy.h"
#include "spellbook.h"
#include "widgets/fileselect.h"
#include "widgets/floatslider.h"
#include "widgets/floatedit.h"
#include "widgets/nifview.h"
#include "widgets/refrbrowser.h"
#include "widgets/inspect.h"
#include "widgets/xmlcheck.h"

#ifdef FSENGINE
#include <fsengine/fsmanager.h>
#endif

#include <QAction>
#include <QApplication>
#include <QByteArray>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDockWidget>
#include <QFontDialog>
#include <QHeaderView>
#include <QMenu>
#include <QMenuBar>
#include <QProgressBar>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>
#include <QWidgetAction>

#include <QProcess>
#include <QStyleFactory>


NifSkope * NifSkope::createWindow( const QString & fname )
{
	NifSkope * skope = new NifSkope;
	skope->setAttribute( Qt::WA_DeleteOnClose );
	skope->restoreUi();
	skope->show();
	skope->raise();

	// Example Dark style
	//QApplication::setStyle( QStyleFactory::create( "Fusion" ) );
	//QPalette darkPalette;
	//darkPalette.setColor( QPalette::Window, QColor( 53, 53, 53 ) );
	//darkPalette.setColor( QPalette::WindowText, Qt::white );
	//darkPalette.setColor( QPalette::Base, QColor( 25, 25, 25 ) );
	//darkPalette.setColor( QPalette::AlternateBase, QColor( 53, 53, 53 ) );
	//darkPalette.setColor( QPalette::ToolTipBase, Qt::white );
	//darkPalette.setColor( QPalette::ToolTipText, Qt::white );
	//darkPalette.setColor( QPalette::Text, Qt::white );
	//darkPalette.setColor( QPalette::Button, QColor( 53, 53, 53 ) );
	//darkPalette.setColor( QPalette::ButtonText, Qt::white );
	//darkPalette.setColor( QPalette::BrightText, Qt::red );
	//darkPalette.setColor( QPalette::Link, QColor( 42, 130, 218 ) );
	//darkPalette.setColor( QPalette::Highlight, QColor( 42, 130, 218 ) );
	//darkPalette.setColor( QPalette::HighlightedText, Qt::black );
	//qApp->setPalette( darkPalette );
	//qApp->setStyleSheet( "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }" );

	if ( !fname.isEmpty() ) {
		skope->openFile( fname );
	}

	return skope;
}

void NifSkope::initActions()
{
	aSanitize = ui->aSanitize;
	aList = ui->aList;
	aHierarchy = ui->aHierarchy;
	aCondition = ui->aCondition;
	aRCondition = ui->aRCondition;
	aSelectFont = ui->aSelectFont;

	//ui->aSave->setShortcut( QKeySequence::Save ); // Bad idea, goes against previous shortcuts
	//ui->aSaveAs->setShortcut( QKeySequence::SaveAs ); // Bad idea, goes against previous shortcuts
	ui->aWindow->setShortcut( QKeySequence::New );

	connect( ui->aOpen, &QAction::triggered, this, &NifSkope::open );
	connect( ui->aSave, &QAction::triggered, this, &NifSkope::save );  
	connect( ui->aSaveAs, &QAction::triggered, this, &NifSkope::saveAs );

	// TODO: Assure Actions and Scene state are synced
	// Set Data for Actions to pass onto Scene when clicking
	/*	ShowAxes = 0x1,
		ShowGrid = 0x2, // Not implemented
		ShowNodes = 0x4,
		ShowCollision = 0x8,
		ShowConstraints = 0x10,
		ShowMarkers = 0x20, // Not implemented
		ShowDoubleSided = 0x40, // Not implemented
		ShowVertexColors = 0x80,
		UseTextures = 0x100,
		UseShaders = 0x200, // Not implemented
		DoBlending = 0x400, // Not implemented
		DoMultisampling = 0x800 // Not implemented
		DoLighting = 0x1000
	*/

	ui->aShowAxes->setData( 0x1 );
	ui->aShowNodes->setData( 0x4 );
	ui->aShowCollision->setData( 0x8 );
	ui->aShowConstraints->setData( 0x10 );
	ui->aVertexColors->setData( 0x80 );
	ui->aTextures->setData( 0x100 );
	ui->aLighting->setData( 0x1000 );

	connect( ui->aShowAxes, &QAction::triggered, ogl->getScene(), &Scene::updateSceneOptions );
	connect( ui->aShowNodes, &QAction::triggered, ogl->getScene(), &Scene::updateSceneOptions );
	connect( ui->aShowCollision, &QAction::triggered, ogl->getScene(), &Scene::updateSceneOptions );
	connect( ui->aShowConstraints, &QAction::triggered, ogl->getScene(), &Scene::updateSceneOptions );
	connect( ui->aTextures, &QAction::triggered, ogl->getScene(), &Scene::updateSceneOptions );
	connect( ui->aVertexColors, &QAction::triggered, ogl->getScene(), &Scene::updateSceneOptions );
	connect( ui->aLighting, &QAction::triggered, ogl->getScene(), &Scene::updateSceneOptions );


	// Setup blank QActions for Recent Files menus
	for ( int i = 0; i < NumRecentFiles; ++i ) {
		recentFileActs[i] = new QAction( this );
		recentFileActs[i]->setVisible( false );
		connect( recentFileActs[i], &QAction::triggered, this, &NifSkope::openRecentFile );
	}


	aList->setChecked( list->model() == nif );
	aHierarchy->setChecked( list->model() == proxy );

	// Allow only List or Tree view to be selected at once
	gListMode = new QActionGroup( this );
	gListMode->addAction( aList );
	gListMode->addAction( aHierarchy );
	gListMode->setExclusive( true );
	connect( gListMode, &QActionGroup::triggered, this, &NifSkope::setListMode );


	connect( aCondition, &QAction::toggled, aRCondition, &QAction::setEnabled );
	connect( aRCondition, &QAction::toggled, tree, &NifTreeView::setRealTime );
	connect( aRCondition, &QAction::toggled, kfmtree, &NifTreeView::setRealTime );

	// use toggled to enable startup values to take effect
	connect( aCondition, &QAction::toggled, tree, &NifTreeView::setEvalConditions );
	connect( aCondition, &QAction::toggled, kfmtree, &NifTreeView::setEvalConditions );

	connect( ui->aAboutNifSkope, &QAction::triggered, aboutDialog, &AboutDialog::show );
	connect( ui->aAboutQt, &QAction::triggered, qApp, &QApplication::aboutQt );

#ifdef FSENGINE
	auto fsm = FSManager::get();
	if ( fsm ) {
		connect( ui->aResources, &QAction::triggered, fsm, &FSManager::selectArchives );
	}
#endif

	connect( ui->aPrintView, &QAction::triggered, ogl, &GLView::saveImage );

#ifdef QT_NO_DEBUG
	ui->aColorKeyDebug->setDisabled( true );
	ui->aColorKeyDebug->setVisible( false );
#else
	connect( ui->aColorKeyDebug, &QAction::triggered, [this]( bool checked ) {
		if ( checked )
			ogl->setDebugMode( GLView::DbgColorPicker );
		else
			ogl->setDebugMode( GLView::DbgNone );

		ogl->update();
	} );
#endif

	connect( ogl, &GLView::clicked, this, &NifSkope::select );
	connect( ogl, &GLView::customContextMenuRequested, this, &NifSkope::contextMenu );
	connect( ogl, &GLView::sigTime, inspect, &InspectView::updateTime );
	connect( ogl, &GLView::paintUpdate, inspect, &InspectView::refresh );
	connect( ogl, &GLView::viewpointChanged, [this]() {
		ui->aViewTop->setChecked( false );
		ui->aViewFront->setChecked( false );
		ui->aViewLeft->setChecked( false );
		ui->aViewUser->setChecked( false );

		ogl->setOrientation( GLView::ViewDefault, false );
	} );

	// Update Inspector widget with current index
	connect( tree, &NifTreeView::sigCurrentIndexChanged, inspect, &InspectView::updateSelection );

	
	// Hide new Settings from Release for the time being
#ifdef QT_NO_DEBUG
	ui->aSettings->setVisible( false );
#endif
}

void NifSkope::initDockWidgets()
{
	dRefr = ui->RefrDock;
	dList = ui->ListDock;
	dTree = ui->TreeDock;
	dInsp = ui->InspectDock;
	dKfm = ui->KfmDock;

	// Hide certain docks by default
	dRefr->toggleViewAction()->setChecked( false );
	dInsp->toggleViewAction()->setChecked( false );
	dKfm->toggleViewAction()->setChecked( false );

	dRefr->setVisible( false );
	dInsp->setVisible( false );
	dKfm->setVisible( false );

	// Set Inspect widget
	dInsp->setWidget( inspect );

	connect( dList->toggleViewAction(), &QAction::triggered, tree, &NifTreeView::clearRootIndex );

}

void NifSkope::initMenu()
{
	// Disable without NIF loaded
	ui->mRender->setEnabled( false );

#ifndef FSENGINE
	if ( ui->aResources ) {
		ui->mOptions->removeAction( ui->aResources );
	}
#endif

	// Populate Toolbars menu with all enabled toolbars
	for ( QObject * o : children() ) {
		QToolBar * tb = qobject_cast<QToolBar *>(o);
		if ( tb && tb->objectName() != "tFile" ) {
			// Do not add tFile to the list
			ui->mToolbars->addAction( tb->toggleViewAction() );
		}
	}

	// Insert SpellBook class before Help
	ui->menubar->insertMenu( ui->menubar->actions().at( 3 ), book );

	ui->mOptions->addActions( Options::actions() );

	// Insert Import/Export menus
	mExport = ui->menuExport;
	mImport = ui->menuImport;

	fillImportExportMenus();
	connect( mExport, &QMenu::triggered, this, &NifSkope::sltImportExport );
	connect( mImport, &QMenu::triggered, this, &NifSkope::sltImportExport );

	for ( int i = 0; i < NumRecentFiles; ++i )
		ui->mRecentFiles->addAction( recentFileActs[i] );


	// Load & Save
	QMenu * mSave = new QMenu( this );
	mSave->setObjectName( "mSave" );

	mSave->addAction( ui->aSave );
	mSave->addAction( ui->aSaveAs );

	QMenu * mOpen = new QMenu( this );
	mOpen->setObjectName( "mOpen" );

	mOpen->addAction( ui->aOpen );

	aRecentFilesSeparator = mOpen->addSeparator();
	for ( int i = 0; i < NumRecentFiles; ++i )
		mOpen->addAction( recentFileActs[i] );

	// Append Menu to tFile actions
	for ( auto child : ui->tFile->findChildren<QToolButton *>() ) {

		if ( child->defaultAction() == ui->aSaveMenu ) {
			child->setMenu( mSave );
			child->setPopupMode( QToolButton::InstantPopup );
		}

		if ( child->defaultAction() == ui->aOpenMenu ) {
			child->setMenu( mOpen );
			child->setPopupMode( QToolButton::InstantPopup );
		}
	}

	updateRecentFileActions();

	// Lighting Menu
	// TODO: Split off into own widget
	auto mLight = lightingWidget();

	// Append Menu to tFile actions
	for ( auto child : ui->tRender->findChildren<QToolButton *>() ) {

		if ( child->defaultAction() == ui->aLightMenu ) {
			child->setStyleSheet( "padding-left: 2px; padding-right: 10px;" );
			child->setMenu( mLight );
			child->setPopupMode( QToolButton::InstantPopup );
		}
	}

}


void NifSkope::initToolBars()
{

	//ui->tFile->setContextMenuPolicy( Qt::NoContextMenu );

	// Disable without NIF loaded
	ui->tRender->setEnabled( false );
	ui->tRender->setContextMenuPolicy( Qt::ActionsContextMenu );

	// Status Bar
	ui->statusbar->setContentsMargins( 0, 0, 0, 0 );
	ui->statusbar->addPermanentWidget( progress );
	
	// TODO: Split off into own widget
	ui->statusbar->addPermanentWidget( filePathWidget( this ) );


	// Render

	QActionGroup * grpView = new QActionGroup( this );
	grpView->addAction( ui->aViewTop );
	grpView->addAction( ui->aViewFront );
	grpView->addAction( ui->aViewLeft );
	grpView->addAction( ui->aViewWalk );
	grpView->setExclusive( true );


	// Animate
	connect( ui->aAnimate, &QAction::toggled, ui->tAnim, &QToolBar::setVisible );
	//connect( ui->aAnimate, &QAction::toggled, ui->tAnim, &QToolBar::setEnabled );
	connect( ui->tAnim, &QToolBar::visibilityChanged, ui->aAnimate, &QAction::setChecked );

	//QActionGroup * grpAnim = new QActionGroup( this );
	//grpAnim->addAction( ui->aAnimLoop );
	//grpAnim->addAction( ui->aAnimSwitch );
	//grpAnim->setExclusive( true );


	/*enum AnimationStates
	{
		AnimDisabled = 0x0,
		AnimEnabled = 0x1,
		AnimPlay = 0x2,
		AnimLoop = 0x4,
		AnimSwitch = 0x8
	};*/

	ui->aAnimate->setData( 0x1 );
	ui->aAnimPlay->setData( 0x2 );
	ui->aAnimLoop->setData( 0x4 );
	ui->aAnimSwitch->setData( 0x8 );

	connect( ui->aAnimate, &QAction::toggled, ogl, &GLView::updateAnimationState );
	connect( ui->aAnimPlay, &QAction::triggered, ogl, &GLView::updateAnimationState );
	connect( ui->aAnimLoop, &QAction::toggled, ogl, &GLView::updateAnimationState );
	connect( ui->aAnimSwitch, &QAction::toggled, ogl, &GLView::updateAnimationState );

	// Animation timeline slider
	auto animSlider = new FloatSlider( Qt::Horizontal, true, true );
	animSlider->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::MinimumExpanding );
	animSlider->setParent( ui->tAnim );
	animSlider->setMinimumWidth( 75 );
	animSlider->setMaximumWidth( 150 );
	connect( ogl, &GLView::sigTime, animSlider, &FloatSlider::set );
	connect( animSlider, &FloatSlider::valueChanged, ogl, &GLView::sltTime );


	FloatEdit * animSliderEdit = new FloatEdit( ui->tAnim );
	//animSliderEdit->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Maximum ); // Qt warning

	connect( ogl, &GLView::sigTime, animSliderEdit, &FloatEdit::set );
	connect( animSliderEdit, static_cast<void (FloatEdit::*)(float)>(&FloatEdit::sigEdited), ogl, &GLView::sltTime );
	connect( animSlider, &FloatSlider::valueChanged, animSliderEdit, &FloatEdit::setValue );
	connect( animSliderEdit, static_cast<void (FloatEdit::*)(float)>(&FloatEdit::sigEdited), animSlider, &FloatSlider::setValue );
	animSlider->addEditor( animSliderEdit );

	// Animations
	animGroups = new QComboBox( ui->tAnim );
	animGroups->setMinimumWidth( 60 );
	animGroups->setSizeAdjustPolicy( QComboBox::AdjustToContents );
	animGroups->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );
	connect( animGroups, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::activated), ogl, &GLView::sltSequence );

	ui->tAnim->addWidget( animSlider );
	auto animGroupsAction = ui->tAnim->addWidget( animGroups );

	connect( ogl, &GLView::sequencesDisabled, ui->tAnim, &QToolBar::hide );

	connect( ogl, &GLView::sequenceStopped, ui->aAnimPlay, &QAction::toggle );
	connect( ogl, &GLView::sequenceChanged, [this]( const QString & seqname ) {
		animGroups->setCurrentIndex( ogl->scene->animGroups.indexOf( seqname ) );
	} );

	connect( ogl, &GLView::sequencesUpdated, [this, animGroupsAction]() {

		ui->tAnim->show();

		animGroups->clear();
		animGroups->addItems( ogl->scene->animGroups );
		animGroups->setCurrentIndex( ogl->scene->animGroups.indexOf( ogl->scene->animGroup ) );

		if ( animGroups->count() == 0 ) {
			animGroupsAction->setVisible( false );
			ui->aAnimSwitch->setVisible( false );
		}
		else {
			ui->aAnimSwitch->setVisible( animGroups->count() != 1 );
			animGroupsAction->setVisible( true );
			animGroups->adjustSize();
		}
			
	} );


	// LOD Toolbar
	QToolBar * tLOD = ui->tLOD;

	QSettings cfg;
	int lodLevel = cfg.value( "GLView/LOD Level", 2 ).toInt();
	cfg.setValue( "GLView/LOD Level", lodLevel );

	QSlider * lodSlider = new QSlider( Qt::Horizontal );
	lodSlider->setFocusPolicy( Qt::StrongFocus );
	lodSlider->setTickPosition( QSlider::TicksBelow );
	lodSlider->setTickInterval( 1 );
	lodSlider->setSingleStep( 1 );
	lodSlider->setMinimum( 0 );
	lodSlider->setMaximum( 2 );
	lodSlider->setValue( lodLevel );

	tLOD->addWidget( lodSlider );
	tLOD->setEnabled( false );

	connect( lodSlider, &QSlider::valueChanged, ogl->getScene(), &Scene::updateLodLevel );
	connect( lodSlider, &QSlider::valueChanged, Options::get(), &Options::sigChanged );
	connect( nif, &NifModel::lodSliderChanged, [tLOD]( bool enabled ) { tLOD->setEnabled( enabled ); tLOD->setVisible( enabled ); } );
}

void NifSkope::initConnections()
{
	connect( nif, &NifModel::beginUpdateHeader, this, &NifSkope::enableUi );

	connect( this, &NifSkope::beginLoading, [this]() {
		setEnabled( false );
		ui->tAnim->setEnabled( false );

		progress->setVisible( true );
		progress->reset();
	} );

	connect( this, &NifSkope::completeLoading, this, &NifSkope::onLoadComplete );
}


QMenu * NifSkope::lightingWidget()
{
	QMenu * mLight = new QMenu( this );
	mLight->setObjectName( "mLight" );

	mLight->setStyleSheet(
		R"qss(#mLight { background: #f5f5f5; padding: 8px; border: 1px solid #CCC; })qss"
		);

	auto chkLighting = new QToolButton( mLight );
	chkLighting->setObjectName( "chkLighting" );
	chkLighting->setCheckable( true );
	chkLighting->setChecked( true );
	chkLighting->setDefaultAction( ui->aLighting );
	chkLighting->setIconSize( QSize( 18, 18 ) );
	chkLighting->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
	//chkLighting->setText( "    " + ui->aLighting->text() ); // Resets during toggle
	//chkLighting->setStatusTip( ui->aLighting->statusTip() ); Doesn't work
	chkLighting->setStyleSheet( R"qss( 
		#chkLighting { padding: 5px; padding-left: 18px; } /* Hackish padding */
	)qss" );

	auto lightingOptionsWidget = new QWidgetAction( mLight );

	auto optionsWidget = new QWidget;
	optionsWidget->setContentsMargins( 0, 0, 0, 0 );
	auto optionsLayout = new QHBoxLayout;
	optionsLayout->setContentsMargins( 0, 0, 0, 0 );
	optionsWidget->setLayout( optionsLayout );


	auto chkFrontal = new QToolButton( mLight );
	chkFrontal->setObjectName( "chkFrontal" );
	chkFrontal->setCheckable( true );
	chkFrontal->setChecked( true );
	chkFrontal->setDefaultAction( ui->aFrontalLight );
	//chkFrontal->setIconSize( QSize( 16, 16 ) );
	//chkFrontal->setToolButtonStyle( Qt::ToolButtonTextBesideIcon );
	//chkFrontal->setStatusTip( ui->aFrontalLight->statusTip() ); Doesn't work
	chkFrontal->setStyleSheet( R"qss( #chkFrontal { padding: 5px; } )qss" );

	auto sldDeclination = new QSlider( Qt::Horizontal, chkFrontal );
	sldDeclination->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Maximum );
	sldDeclination->setRange( -360, 360 );
	sldDeclination->setSingleStep( 45 );
	sldDeclination->setTickInterval( 180 );
	sldDeclination->setTickPosition( QSlider::TicksBelow );
	sldDeclination->setValue( 0 );  // Render Settings/Light0/Declination
	//sldDeclination->setDisabled( true );

	auto sldPlanarAngle = new QSlider( Qt::Horizontal, chkFrontal );
	sldPlanarAngle->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Maximum );
	sldPlanarAngle->setRange( -360, 360 );
	sldPlanarAngle->setSingleStep( 45 );
	sldPlanarAngle->setTickInterval( 180 );
	sldPlanarAngle->setTickPosition( QSlider::TicksBelow );
	sldPlanarAngle->setValue( 0 );  // Render Settings/Light0/Planar Angle
	//sldPlanarAngle->setDisabled( true );

	auto chkTextures = new QToolButton( mLight );
	chkTextures->setObjectName( "chkTextures" );
	chkTextures->setCheckable( true );
	chkTextures->setChecked( true );
	chkTextures->setDefaultAction( ui->aTextures );
	chkTextures->setIconSize( QSize( 16, 16 ) );
	//chkTextures->setStatusTip( ui->aTextures->statusTip() ); Doesn't work
	chkTextures->setStyleSheet( R"qss( #chkTextures { padding: 3px 2px 2px 3px; } )qss" );

	auto chkVertexColors = new QToolButton( mLight );
	chkVertexColors->setObjectName( "chkVertexColors" );
	chkVertexColors->setCheckable( true );
	chkVertexColors->setChecked( true );
	chkVertexColors->setDefaultAction( ui->aVertexColors );
	chkVertexColors->setIconSize( QSize( 16, 16 ) );
	//chkVertexColors->setStatusTip( ui->aTextures->statusTip() ); Doesn't work
	chkVertexColors->setStyleSheet( R"qss( #chkVertexColors { padding: 3px 2px 2px 3px; } )qss" );


	optionsLayout->addWidget( chkFrontal );
	optionsLayout->addWidget( chkTextures );
	optionsLayout->addWidget( chkVertexColors );

	lightingOptionsWidget->setDefaultWidget( optionsWidget );

	auto lightingGroup = new QGroupBox( mLight );
	lightingGroup->setObjectName( "lightingGroup" );
	lightingGroup->setContentsMargins( 0, 0, 0, 0 );
	lightingGroup->setStyleSheet( R"qss( #lightingGroup { padding: 0; border: none; } )qss" );
	lightingGroup->setDisabled( true );

	auto lightingGroupVbox = new QVBoxLayout;
	lightingGroupVbox->setContentsMargins( 0, 0, 0, 0 );
	lightingGroupVbox->setSpacing( 0 );
	lightingGroup->setLayout( lightingGroupVbox );

	lightingGroupVbox->addWidget( sldDeclination );
	lightingGroupVbox->addWidget( sldPlanarAngle );

	// Disable lighting sliders when Frontal
	connect( chkFrontal, &QToolButton::toggled, lightingGroup, &QGroupBox::setDisabled );

	// Disable Frontal checkbox (and sliders) when no lighting
	connect( chkLighting, &QToolButton::toggled, chkFrontal, &QToolButton::setEnabled );
	connect( chkLighting, &QToolButton::toggled, [lightingGroup, chkFrontal]( bool checked ) {
		if ( !chkFrontal->isChecked() ) {
			// Don't enable the sliders if Frontal is checked
			lightingGroup->setEnabled( checked );
		}
	} );

	// Inform ogl of changes
	//connect( chkLighting, &QCheckBox::toggled, ogl, &GLView::lightingToggled );
	connect( sldDeclination, &QSlider::valueChanged, ogl, &GLView::declinationChanged );
	connect( sldPlanarAngle, &QSlider::valueChanged, ogl, &GLView::planarAngleChanged );
	connect( chkFrontal, &QToolButton::toggled, ogl, &GLView::frontalLightToggled );


	// Set up QWidgetActions so they can be added to a QMenu
	auto lightingWidget = new QWidgetAction( mLight );
	lightingWidget->setDefaultWidget( chkLighting );

	auto lightingAngleWidget = new QWidgetAction( mLight );
	lightingAngleWidget->setDefaultWidget( lightingGroup );

	mLight->addAction( lightingWidget );
	mLight->addAction( lightingAngleWidget );
	mLight->addAction( lightingOptionsWidget );

	return mLight;
}


QWidget * NifSkope::filePathWidget( QWidget * parent )
{
	// Show Filepath of loaded NIF
	auto filepathWidget = new QWidget( this );
	filepathWidget->setObjectName( "filepathStatusbarWidget" );
	auto filepathLayout = new QHBoxLayout( filepathWidget );
	filepathWidget->setLayout( filepathLayout );
	filepathLayout->setContentsMargins( 0, 0, 0, 0 );
	auto labelFilepath = new QLabel( "", filepathWidget );
	labelFilepath->setMinimumHeight( 16 );

	filepathLayout->addWidget( labelFilepath );

	// Navigate to Filepath
	auto navigateToFilepath = new QPushButton( "", filepathWidget );
	navigateToFilepath->setFlat( true );
	navigateToFilepath->setIcon( QIcon( ":btn/loadDark" ) );
	navigateToFilepath->setIconSize( QSize( 16, 16 ) );
	navigateToFilepath->setStatusTip( tr( "Show in Explorer" ) );

	filepathLayout->addWidget( navigateToFilepath );

	filepathWidget->setVisible( false );

	// Show Filepath on successful NIF load
	connect( this, &NifSkope::completeLoading, [this, filepathWidget, labelFilepath]( bool success ) {
		filepathWidget->setVisible( success );
		labelFilepath->setText( currentFile );
		//ui->statusbar->showMessage( tr("File Loaded Successfully"), 3000 );
	} );

	// Navigate to NIF in Explorer
	connect( navigateToFilepath, &QPushButton::clicked, [this]() {
#ifdef Q_OS_WIN
		QStringList args;
		args << "/select," << QDir::toNativeSeparators( currentFile );
		QProcess::startDetached( "explorer", args );
#endif
	} );


	return filepathWidget;
}


void NifSkope::onLoadComplete( bool success )
{
	QApplication::restoreOverrideCursor();

	setEnabled( true ); // IMPORTANT! Re-enables the main window.

	int timeout = 2500;
	if ( success ) {
		
		ogl->setOrientation( GLView::ViewFront );

		enableUi();

	} else {
		timeout = 0;

		// File failed in some way, remove from Current Files
		clearCurrentFile();

		// Reset
		currentFile = "";
		setWindowFilePath( "" );
		progress->reset();
	}

	// Hide Progress Bar
	QTimer::singleShot( timeout, progress, SLOT( hide() ) );
}


void NifSkope::enableUi()
{
	// Re-enable toolbars, actions, and menus
	ui->aSaveMenu->setEnabled( true );
	ui->aSave->setEnabled( true );
	ui->aSaveAs->setEnabled( true );
	ui->aResetBlockDetails->setEnabled( true );

	ui->mRender->setEnabled( true );
	ui->tAnim->setEnabled( true );
	animGroups->clear();


	ui->tRender->setEnabled( true );

	// We only need to enable the UI once, disconnect
	disconnect( nif, &NifModel::beginUpdateHeader, this, &NifSkope::enableUi );
}

void NifSkope::saveUi() const
{
	QSettings settings;
	// TODO: saveState takes a version number which can be incremented between releases if necessary
	settings.setValue( "UI/Window State", saveState( 0x073 ) );
	settings.setValue( "UI/Window Geometry", saveGeometry() );

	settings.setValue( "File/Auto Sanitize", aSanitize->isChecked() );

	settings.setValue( "UI/List Mode", (gListMode->checkedAction() == aList ? "list" : "hierarchy") );
	settings.setValue( "UI/Hide Mismatched Rows", aCondition->isChecked() );
	settings.setValue( "UI/Realtime Condition Updating", aRCondition->isChecked() );

	settings.setValue( "UI/List Header", list->header()->saveState() );
	settings.setValue( "UI/Tree Header", tree->header()->saveState() );
	settings.setValue( "UI/Kfmtree Header", kfmtree->header()->saveState() );

	settings.setValue( "GLView/Enable Animations", ui->aAnimate->isChecked() );
	//settings.setValue( "GLView/Play Animation", ui->aAnimPlay->isChecked() );
	//settings.setValue( "GLView/Loop Animation", ui->aAnimLoop->isChecked() );
	//settings.setValue( "GLView/Switch Animation", ui->aAnimSwitch->isChecked() );
	settings.setValue( "GLView/Perspective", ui->aViewPerspective->isChecked() );

	Options::get()->save();
}


void NifSkope::restoreUi()
{
	QSettings settings;
	restoreGeometry( settings.value( "UI/Window Geometry" ).toByteArray() );
	restoreState( settings.value( "UI/Window State" ).toByteArray(), 0x073 );

	aSanitize->setChecked( settings.value( "File/Auto Sanitize", true ).toBool() );

	if ( settings.value( "UI/List Mode", "hierarchy" ).toString() == "list" )
		aList->setChecked( true );
	else
		aHierarchy->setChecked( true );

	setListMode();

	aCondition->setChecked( settings.value( "UI/Hide Mismatched Rows", false ).toBool() );
	aRCondition->setChecked( settings.value( "UI/Realtime Condition Updating", false ).toBool() );

	list->header()->restoreState( settings.value( "UI/List Header" ).toByteArray() );
	tree->header()->restoreState( settings.value( "UI/Tree Header" ).toByteArray() );
	kfmtree->header()->restoreState( settings.value( "UI/Kfmtree Header" ).toByteArray() );

	ui->aAnimate->setChecked( settings.value( "GLView/Enable Animations", true ).toBool() );
	//ui->aAnimPlay->setChecked( settings.value( "GLView/Play Animation", true ).toBool() );
	//ui->aAnimLoop->setChecked( settings.value( "GLView/Loop Animation", true ).toBool() );
	//ui->aAnimSwitch->setChecked( settings.value( "GLView/Switch Animation", true ).toBool() );

	auto isPersp = settings.value( "GLView/Perspective", true ).toBool();
	ui->aViewPerspective->setChecked( isPersp );

	ogl->setProjection( isPersp );

	QVariant fontVar = settings.value( "UI/View Font" );

	if ( fontVar.canConvert<QFont>() )
		setViewFont( fontVar.value<QFont>() );

	// Modify UI settings that cannot be set in Designer
	tabifyDockWidget( ui->InspectDock, ui->KfmDock );
}

void NifSkope::setViewFont( const QFont & font )
{
	list->setFont( font );
	QFontMetrics metrics( list->font() );
	list->setIconSize( QSize( metrics.width( "000" ), metrics.lineSpacing() ) );
	tree->setFont( font );
	tree->setIconSize( QSize( metrics.width( "000" ), metrics.lineSpacing() ) );
	kfmtree->setFont( font );
	kfmtree->setIconSize( QSize( metrics.width( "000" ), metrics.lineSpacing() ) );
	ogl->setFont( font );
}

void NifSkope::resizeDone()
{
	isResizing = false;
	qDebug() << "resizeDone" << isResizing;
	//qDebug() << sender();

	ogl->setUpdatesEnabled( true );
	ogl->setDisabled( false );
	// Testing of parent widget idea
	//ogl->setVisible( true );

	ogl->scene->animate = true;
	ogl->update();
	ogl->resizeGL( ogl->width(), ogl->height() );
}


bool NifSkope::eventFilter( QObject * o, QEvent * e )
{
	// TODO: This doesn't seem to be doing anything extra
	//if ( e->type() == QEvent::Polish ) {
	//	QTimer::singleShot( 0, this, SLOT( overrideViewFont() ) );
	//}

	// Testing of parent widget idea
	//if ( o->objectName() == "centerWidget" ) {
	//	// Paint stored framebuffer over GLView while resizing
	//	if ( isResizing && e->type() == QEvent::Paint ) {
	//		//qDebug() << "isResizing Paint";
	//		QPainter painter;
	//		painter.begin( static_cast<QWidget *>(o) );
	//		painter.drawImage( QRect( 0, 0, painter.device()->width(), painter.device()->height() ), buf );
	//		painter.end();
	//
	//		return true;
	//	}
	//	return QMainWindow::eventFilter( o, e );
	//}

	// Filter GLView
	auto obj = qobject_cast<GLView *>(o);
	if ( !obj )
		return QMainWindow::eventFilter( o, e );

	// Turn off animation
	// Grab framebuffer
	// Begin resize timer
	// Block all Resize Events to GLView
	if ( e->type() == QEvent::Resize ) {

		if ( !isResizing  && !resizeTimer->isActive() ) {
			ogl->scene->animate = false;
			ogl->updateGL();
			buf = ogl->grabFrameBuffer();

			ogl->setUpdatesEnabled( false );

			// Testing of parent widget idea
			//ogl->setVisible( false );

			ogl->setDisabled( true );

			isResizing = true;
			resizeTimer->start( 300 );
		}

		return true;
	}

	// Paint stored framebuffer over GLView while resizing
	if ( isResizing && e->type() == QEvent::Paint ) {
		//qDebug() << "isResizing Paint";
		QPainter painter;
		painter.begin( static_cast<QWidget *>(o) );
		painter.drawImage( QRect( 0, 0, painter.device()->width(), painter.device()->height() ), buf );
		painter.end();

		return true;
	}

	// Doesn't work. Won't update after resize finishes
	//if ( isResizing && resizeTimer->isActive() && (e->type() == QEvent::UpdateLater || e->type() == QEvent::UpdateRequest) ) {
	//	qDebug() << "isResizing UpdateLater";
	//	return true;
	//}

	return QMainWindow::eventFilter( o, e );
}


/*
* Slots
* **********************
*/


void NifSkope::contextMenu( const QPoint & pos )
{
	QModelIndex idx;
	QPoint p = pos;

	if ( sender() == tree ) {
		idx = tree->indexAt( pos );
		p = tree->mapToGlobal( pos );
	} else if ( sender() == list ) {
		idx = list->indexAt( pos );
		p = list->mapToGlobal( pos );
	} else if ( sender() == ogl ) {
		idx = ogl->indexAt( pos );
		p = ogl->mapToGlobal( pos );
	} else {
		return;
	}

	while ( idx.model() && idx.model()->inherits( "NifProxyModel" ) ) {
		idx = qobject_cast<const NifProxyModel *>(idx.model())->mapTo( idx );
	}

	SpellBook contextBook( nif, idx, this, SLOT( select( const QModelIndex & ) ) );
	contextBook.exec( p );
}

void NifSkope::overrideViewFont()
{
	QSettings settings;
	QVariant var = settings.value( "UI/View Font" );

	if ( var.canConvert<QFont>() ) {
		setViewFont( var.value<QFont>() );
	}
}


/*
* Automatic Slots
* **********************
*/


void NifSkope::on_aLoadXML_triggered()
{
	NifModel::loadXML();
	KfmModel::loadXML();
}

void NifSkope::on_aReload_triggered()
{
	if ( NifModel::loadXML() ) {
		reload();
	}
}

void NifSkope::on_aSelectFont_triggered()
{
	bool ok;
	QFont fnt = QFontDialog::getFont( &ok, list->font(), this );

	if ( !ok )
		return;

	setViewFont( fnt );
	QSettings settings;
	settings.setValue( "UI/View Font", fnt );
}

void NifSkope::on_aWindow_triggered()
{
	createWindow();
}

void NifSkope::on_aShredder_triggered()
{
	TestShredder::create();
}

void NifSkope::on_aResetBlockDetails_triggered()
{
	if ( tree )
		tree->clearRootIndex();
}


void NifSkope::on_tRender_actionTriggered( QAction * action )
{

}

void NifSkope::on_aViewTop_triggered( bool wasChecked )
{
	if ( wasChecked ) {
		ogl->setOrientation( GLView::ViewTop );
	}
}

void NifSkope::on_aViewFront_triggered( bool wasChecked )
{
	if ( wasChecked ) {
		ogl->setOrientation( GLView::ViewFront );
	}
}

void NifSkope::on_aViewLeft_triggered( bool wasChecked )
{
	if ( wasChecked ) {
		ogl->setOrientation( GLView::ViewLeft );
	}
}

void NifSkope::on_aViewCenter_triggered()
{
	qDebug() << "Centering";
	ogl->center();
}

void NifSkope::on_aViewFlip_triggered( bool wasChecked )
{
	qDebug() << "Flipping";
	ogl->flipOrientation();

}

void NifSkope::on_aViewPerspective_toggled( bool wasChecked ) {
	ogl->setProjection( wasChecked );

	//if ( wasChecked )
	//	ui->aViewPerspective->setText( tr( "Perspective" ) );
	//else
	//	ui->aViewPerspective->setText( tr( "Orthographic" ) );
}

void NifSkope::on_aViewWalk_triggered( bool wasChecked )
{
	if ( wasChecked ) {
		ogl->setOrientation( GLView::ViewWalk );
	}
}


void NifSkope::on_aViewUserSave_triggered( bool wasChecked )
{ 
	ogl->saveUserView();
	ui->aViewUser->setChecked( true );
}


void NifSkope::on_aViewUser_toggled( bool wasChecked )
{
	if ( wasChecked ) {
		ogl->setOrientation( GLView::ViewUser, false );
		ogl->loadUserView();
	}
}

void NifSkope::on_aSettings_triggered()
{
	settingsDlg->show();
	qApp->setActiveWindow( settingsDlg->window() );
}