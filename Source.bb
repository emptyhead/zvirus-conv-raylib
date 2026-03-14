Graphics3D 640,480,16,1
AmbientLight  255,255,255 
SetFont LoadFont("Veranda",16)
SeedRnd MilliSecs()

;for original control
Global MouseMin# , MouseMax# 
Global MouseSens# = 0.0
Global Mx#, Mz#
Global MousePivot = CreatePivot()
Global MouseYaw = CreatePivot()

;Global Light = CreateLight() : RotateEntity light, 30,-45,0
Const ViewCollisions = False 
Global ActiveParticles
Global CrossHairON = True
Global AlliesON, AlliesGO
Global GridSize# = 0
Global Gravity#
Global FadeStatus# = 1.0
Global NX#,NY#,NZ#
Global JX#,JZ#,DP#,DY#,Shoot, Launch, Bomb, Infect, Thrust#, Attract, ShootAt
Global Tractor
Global Scores
Global HGW# = GraphicsWidth()*.5
Global HGH# = GraphicsHeight()*.5
Global GW# = GraphicsWidth()
Global GH# = GraphicsHeight()
Const OScale#= .0625
Const FPS = 30
Global MaxPlayers 
Global MaxHeight =150
Const MaxParticles = 750
Const MaxScoreTags = 10
Const Size = 256
Const GroundObjects = 11
Const FlyingObjects = 20
Const ParticleTemplates = 21
Const SoundGroups  = 9
Const CONTROLdemo = 0
Const CONTROLmouse = 1
Const CONTROLoriginal= 2
Const CONTROLjoypad = 3
Global CONTROLcurrent 
Const CAMERAoriginal = 0
Const CAMERAchase = 1
Const CAMERArear = 2
Global CAMERAcurrent = CAMERAoriginal 
Global Camera, CameraYaw, Cam
Global Shadow, ShadowTexture , ShadowSize
Global Timer = CreateTimer( FPS )
Global Pivot = CreatePivot()
Global Chase = CreatePivot()
Global Temp1 = CreatePivot()
Global Temp2 = CreatePivot()
Global Rotate, Ripple
Global ParticleNext, ParticleTemplateMesh, ParticleMesh
Global BulletNext
Global ScoreTagNext
Global Score, Missiles, Lives, Wave , LastWave, Difficulty, BestScore = 1000, BonusLife
Global Enemies, AreaTotal, AreaInfected
Global BonusTimer#
Global QUIT, GAMEOVER, WAVECOMPLETE

Global SOUNDmenu
Global SOUNDthunder, CHANNELthunder
Global SOUNDwind, CHANNELwind
Global SOUNDshoot, CHANNELshoot
Global SOUNDsmall, CHANNELsmall
Global SOUNDexplode, CHANNELexplode
Global SOUNDsplat, CHANNELsplat
Global SOUNDlaunch, CHANNELlaunch
Global SOUNDtimer = FPS * 10
Global SOUNDallie, SOUNDbabble
Global SOUNDfanfare, SOUNDfirework
Global SOUNDsplash, CHANNELsplash, SPLASHtimer

Type SoundGroupType
	Field Sound
	Field Other
	Field Channel
	Field Distance#
	Field Volume#
	Field Id
	Field Index
End Type

Type MenuType
	Field model
	Field texture
	Field id
	Field y_scale
	Field y_spacing
	Field x_spacing
	Field min_option
	Field max_option
	Field current
	Field name$[5]
	Field sub_min[5]
	Field sub_max[5]
	Field sub_current[5]
	Field sub_name$[5]
End Type

Type ScoreTagType
	Field Sprite
	Field x#, y#, z#
	Field life#
End Type

Type ParticleTemplateType
	Field Splash
	Field Alpha#
	Field r1,r2
	Field g1,g2
	Field b1,b2
	Field s1#,s2#
	Field vx1#,vx2#
	Field vy1#,vy2#
	Field vz1#,vz2#
	Field Weight#
	Field Fade#
End Type

Type ParticleType
	Field Id
	Field Index
	Field InView
	Field Vertex
	Field x#,y#,z#
	Field vx#,vy#,vz#
	Field r,g,b
	Field life#
	Field size#
End Type

Type GroundObjectType
	Field Model[2]
	Field height#
End Type

Type FlyingObjectType
	Field Model
	Field Radius#
	Field Name$
	Field Points
	Field Tag
	Field Thrust#
	Field Momentum#
	Field FireRate
	Field MissileRate
	Field ThrustRate
	Field Damage
	Field SpinSpeed
	Field engine
	Field PitchMin#
	Field PitchMax#
	Field TurnSpeed#
	Field Range
	Field DeadTimer
	Field r,g,b	
	Field CruiseHeight#	
	Field SoundGroup
End Type

Type GridType
	Field Divs
	Field Divg
	Field View
	Field Cull
	Field Model
	Field S
End Type

Type TerrainType
	Field LandIndex
	Field LandHeight#
	Field ObjectIndex
	Field ObjectHeight#
	Field ObjectStatus
	Field ObjectCycle
	Field ObjectYaw
	Field ObjectPitch#
	Field ObjectSway#
	Field r[1],g[1],b[1]
	Field argb[1]
	Field LandInfected
	Field LandHidden
	Field Wavefactor#
End Type

Type HudType
	Field Camera
	Field Model
	Field Pivot
	Field Display
	Field Map
	Field Fuel
	Field Height
	Field Bonus
	Field Title
	Field Dot
	Field Arrow
	Field CrossHair
	Field OnTarget#
	Field Fade
End Type

Type PlayerType
	Field model		;mesh
	Field dot			;radar dot
	Field AI			;ai type - flying object type
	Field AIcounter	;counter for ai updates
	Field x#,y#,z#    	;world coords
	Field vx#,vy#,vz# 	;velocity vector
	Field thrustcounter;thrust particle reload
	Field reload		;reload status
	Field pitch#		;pitch angle
	Field yaw#		;yaw angle
	Field spin#		;spin angle
	Field fuel#		;fuel
	Field InView		;in grid view
	Field dead		;dead
	Field target		;target
	Field index		;0=allie, 1=enemy, 2=missile, 3=Monster
	Field id			;craft number
	Field child[2] 		;children of craft to launch
	Field counter 		;missile reload status
	Field spread		;bullet spread
	Field gun		;gun pivot for spread & repulsor
	Field ChaseTimer	;timer so chases last less time before another target looked for
End Type

Global Grid.GridType  = New GridType	
Global HUD.HudType = New HudType
Dim SoundGroup.SoundGroupType( SoundGroups )
Dim Terrain.TerrainType( size-1, size-1 )
Dim GroundObject.GroundObjectType ( GroundObjects )
Dim FlyingObject.FlyingObjectType ( FlyingObjects )
Dim Player.PlayerType ( 100 )
Dim Particle.ParticleType( MaxParticles )
Dim ParticleTemplate.ParticleTemplateType( ParticleTemplates )
Dim ScoreTag.ScoreTagType( MaxScoreTags )
Dim MENU.MenuType( 4 )
Dim ParticleLUP#( 2, 5 , 720 )
Dim DistanceLUP( 100 , 100 )
Dim MAPchange( 2, 5000 )
Global MAPcounter

;------------------------------------
.MAINroutine
;------------------------------------

GAMEinit()

Repeat
	GAMEstart()
	Repeat
		WAVEstart()
		Repeat 
			WaitTimer Timer
			PLAYERupdate()
			TERRAINupdate()
			CAMERAupdate()
			PARTICLEupdate()
			SCOREtagupdate()
			MAPupdate()
			RenderWorld()
					
			If CONTROLcurrent = CONTROLoriginal Text 605,465,MouseSens
			
			Flip
			GAMEupdate()
		Until GAMEOVER Or WAVECOMPLETE Or QUIT
		If WAVECOMPLETE And ( Not GAMEOVER ) WAVEend()
	Until GAMEOVER Or QUIT
	GAMEend()
Forever

;------------------------------------
;------------------------------------

;=================================================
;=================================================
;=================================================

Function WAVEstart()

	Wave = Wave + 1
	Players = 0
	Enemies = 0
	AlliesON = False
	AlliesRND = Rand( -180,180)
	AlliesLEFT = 0
	AlliesGO = False
	
	;free all existing player models and reset all to dead
	EntityParent HUD\crosshair, 0
	For p.PlayerType = Each PlayerType
		If p\AI = 18 And p\Dead=0 AlliesLEFT = AlliesLEFT + 1
		p\Dead = -1
		If p\Model > 0
			FreeEntity p\Model
			FreeEntity p\Dot
		EndIf
		p\Model = 0
		p\Dot = 0
	Next
	
	Restore WaveData
		
	For WaveNo = 1 To Wave
		
		Read Map$
		If Map$<>"" And WaveNo = Wave 
			TERRAINload( Map$+".bmp" )
			If LastWave = 0 AlliesLEFT = 0
		EndIf
				
		For AI = 0 To 18
		
			Read Number
			
			If WaveNo = Wave
				
				;allies still alive from last wave
				If AI = 18 
					Number = Number + AlliesLEFT
					If Number > 0 AlliesON = True
				EndIf
			
				For l = 1 To Number
					PLAYERreset( Players , AI )
					
					;Allies
					If AI = 18
						angle# = AlliesRND + ( Float(l) * 360.0 ) / Float(Number)
						Player( Players)\x  = player(0)\x + Cos( Angle ) * 4.5
						Player( Players)\z = player(0)\z + Sin( Angle ) * 4.5
						Player( Players)\y = -1
						Player( Players)\yaw = 270-Angle
					EndIf
					
					Enemies = Enemies + ( player( Players )\index = 1 )
		
					Child = 0
					Select AI
						Case 0  	Child=20
						Case 10 Child=19
						Case 11 Child=4
						Case 12 Child=5
						Case 13 Child=6
						Case 14 Child=7
						Case 15	Child = 16
						Case 16 Child=19
					End Select
					
					If Child > 0
						ParentPlayer = Players
						For w = 0 To 2
							Players = Players + 1
							PLAYERreset( Players , Child , -1 )
							player( ParentPlayer )\child[ w ] = Players
							
							;ELiteGuard
							If AI = 15 
								NewPlayer = Players 
								For v = 0 To 2
									Players = Players + 1
									PLAYERreset( Players, 19, -1 )
									player( NewPlayer )\child[v] = Players
								Next
							EndIf
						Next
					EndIf
					
					Players = Players + 1
					
					If players > 100 RuntimeError "Too Many Players"
		
				Next
				
			EndIf
				
		Next
		
	Next
	
	LastWave = Wave
	MaxPlayers = Players - 1
	
	;reset particles
	For b.ParticleType = Each ParticleType
		b\life = 0
	Next
	PARTICLEupdate()
	
	;reset scoretags
	For st.ScoreTagType = Each ScoreTagType
		st\life =.01
	Next
	SCOREtagupdate()
		
	Cam = 0
	BonusTimer = 0
	WAVECOMPLETE= False
	GAMEOVER = False
	PLAYERdistanceinit()
	PLAYERupdate()
	FADE(1,.025)
	SOUNDon(True)
	
	If AlliesOn > 0 PlaySound( SOUNDAllie )
	MoveMouse HGW,HGH
	
	If Wave = 22 Enemies = 15 * FPS

End Function

;=================================================
;=================================================
;=================================================

Function WAVEend()

	SOUNDon( False )
			
	If Wave = 22 
		GAMEOVER = True 
	Else
		WAVEbonus = LIMIT ( AreaTotal*.5 - AreaInfected,0,size*size) / 16
		Score = Score + WAVEbonus
		menu(1)\sub_name[1] = Str$( AreaInfected )
		menu(1)\sub_name[2] = Str$( AreaTotal - AreaInfected )
		menu(1)\sub_name[3] = Str$( WAVEbonus )
		MENUshow( MENU(1) , False )
		FADE(0,-.025 )
	EndIf
	
End Function

;=================================================
;=================================================
;=================================================

Function COLLISIONground( sx#, sy#, sz#, InView, IsPlayer, Index )

	For fx=0 To 1
		cx = wrap( Floor(sx) + fx , size )
		For fz = 0 To 1
			cz = wrap ( Floor(sz) + fz, size )	
			t.TerrainType = Terrain( cx, cz )
			If t\ObjectIndex > 0 And t\ObjectStatus < 2
				t\ObjectHeight = t\LandHeight
				If t\ObjectIndex =8 MAPadd( 1, cx , cz )
				If t\ObjectIndex=11 And Index = 0
					player(0)\spread = player(0)\spread+1
					BonusTimer =FPS*30
				EndIf
				If t\ObjectIndex < 5 And index = 0
					If t\ObjectStatus = 1 SCOREtagadd( sx, sy, sz, 1, 40 )
					If t\ObjectStatus = 0 SCOREtagadd( sx, sy, sz, 0, -40 )
				EndIf
				t\ObjectStatus = 2
				
				CHANNELsmall = PlaySound ( SOUNDsmall )
				If InView And (Not IsPlayer ) 
					PARTICLEnew( Grid\Model , 13 , sx, sy , sz, 0,0,0,20 )
				EndIf
			EndIf	
		Next
	Next

End Function

;=================================================
;=================================================
;=================================================

Function COLLISIONbullet( p.ParticleType )

	For c.PlayerType = Each PlayerType
		If ( c\dead=0 ) And ( p\index <> c\index )
			PositionEntity Temp1,c\x, c\y,c\z
			PositionEntity Temp2, p\x, p\y, p\z
			d# = EntityDistance ( Temp1, Temp2 )
			F.FlyingObjectType = FlyingObject( c\ai )
			If d < F\radius 
				p\life=0
				c\fuel = c\fuel - F\damage
				If c\InView 
					PARTICLEnew( Grid\Model,14, p\x , p\y, p\z , 0 , 0 , 0, 3 )
					CHANNELsplat = PlaySound ( SOUNDsplat )
					ChannelPitch CHANNELsplat, 10000 + c\Ai * 1000
				EndIf
					
				If c\fuel <=0 
					c\dead=1
					If p\index=0  SCOREtagadd( c\x, c\y, c\z , F\Tag, F\points )
				EndIf
			EndIf	
		EndIf
	Next
			
End Function

;=================================================
;=================================================
;=================================================

Function COLLISIONplayer()

	;flying collisions
	For k = 0 To MaxPlayers-1
		For m = k+1 To MaxPlayers
		
			p.PlayerType = Player( k )
			c.PlayerType = Player(m )
						
			If p\dead=0 And c\dead = 0
			
				Radius# = ( FlyingObject( c\ai )\radius + FlyingObject(p\ai )\radius ) * ( FlyingObject( c\ai )\radius + FlyingObject(p\ai )\radius )
				distance# = PLAYERdistance( p, c )
				DistanceLUP( p\id, c\id )= distance
				DistanceLUP( c\id, p\id ) = distance
				
				If distance < Radius
					
					offset# = Sqr( Radius - distance ) * .5
										
					If p\index <> c\index
						;damage
						c\fuel=c\fuel - 500:If c\fuel <=0 c\dead=1: SCOREtagadd( c\x, c\y,c\z, FlyingObject(c\ai)\tag, FlyingObject(c\ai)\points )
						p\fuel=p\fuel - 500:If p\fuel <=0 p\dead=1: SCOREtagadd( p\x,p\y,p\z, FlyingObject(p\ai)\tag, FlyingObject(p\ai)\points )
					Else
						;bounce 
						p\x = p\x - NX*offset
						p\y = p\y - NY*offset
						p\z = p\z - NZ*offset
						p\vx=p\vx*.75- NX*offset
						p\vy=p\vy*.75-NY*offset
						p\vz=p\vz*.75-NZ*offset
						c\x = c\x + NX*offset
						c\y = c\y + NY*offset
						c\z = c\z + NZ*offset
						c\vx = c\vx*.75+ NX*offset
						c\vy = c\vy*.75+ NY*offset
						c\vz = c\vz*.75 + NZ*offset
					EndIf
				EndIf	
			EndIf
		Next
	Next
		
End Function

;=================================================
;=================================================
;=================================================

Function PLAYERreset( ID , AI , dead = 0)

	p.PlayerType = Player( ID )
	p\ID = ID
	p\ai = AI
	F.FlyingObjectType = FlyingObject ( p\ai )
	
	If p\dot > 0 FreeEntity p\dot
	If p\Model > 0 FreeEntity p\Model
	
	p\dot = CopyEntity(HUD\dot , HUD\model)
	p\Model = CopyEntity ( F\Model )
	
	EntityOrder p\dot,-1000
	EntityFX p\dot,1
	EntityAlpha p\dot,(AI<>17)
	EntityColor p\dot,F\r,F\g,F\b
	ScaleEntity p\dot,F\radius*HGW,F\radius*HGW,HGW 
	
	If dead HideEntity p\Model:HideEntity p\Dot
	If AI > 0
		EntityPickMode p\Model, 1
		EntityRadius p\Model, F\radius * 2.0
		NameEntity p\Model, Str$( F\radius *  2.0 )
	EndIf
	p\Gun = CreatePivot(p\Model)
	p\Spread = 0
	p\Yaw = Rand(5,20) * ( Rand(0,1)*2-1 ) + Rand(1,8)*45
	p\Pitch = 0
	p\index = Sgn( F\points) + (AI=20)*2 + (AI=17)*2
	If AI = 8 Child = CopyMesh( Tractor , p\Model )
				
	Repeat
		r =Rand(64,120)
		a=Rand(0,360)
		p\x = size * .5 -.5 + (p\index<>0) * ( r * Cos(a) )
		p\z = size * .5     + (p\index<>0) * ( r * Sin(a) )
		p\y = TERRAINgetheight( p\x , p\z, 1 ) + .5 * ( p\Index=0 ) + 50 *( p\Index = 1 )
		ok = ( ai <> 17 Or terrain( p\x,p\z)\LandHeight = 0 )
	Until ok=True
	
	If AI = 0 LandingHeight# = p\y
	MoveMouse HGW,HGH
	p\vx=0:p\vy=0:p\vz=0
	p\fuel= 500 + ( ai>10 And ai<15 ) * 500 + (ai=15 ) * 1500
	p\dead = dead
	p\target=-1
	p\spin=0
	p\ChaseTimer = Rand( 10 * FPS )
	
	;test ship collisions
	If ViewCollisions
		sphere = CreateSphere( 8, p\Gun )
		ScaleEntity sphere, F\radius, F\radius, F\radius 
		EntityAlpha sphere, .25
	EndIf
			
End Function

;=================================================
;=================================================
;=================================================

Function PLAYERupdate()

	;arrow
	EntityParent HUD\arrow, player(cam)\dot
	PositionEntity HUD\arrow,0,0,0
	RotateEntity HUD\arrow,0,0,player(cam)\yaw
	EntityParent HUD\arrow,0
	
	For p.PlayerType = Each PlayerType

		F.FlyingObjectType = FlyingObject ( p\ai )

		If p\Dead = 0 
	
			p\reload = p\reload - ( p\reload > 0 )
			p\counter = p\counter - (p\counter > 0 )
			p\thrustcounter = p\thrustcounter - ( p\thrustcounter > 0 )
			GroundHeight# = TERRAINgetheight( p\x, p\z, 0 )
					
			If p\ID = 0 
			
				;control
				Select CONTROLcurrent
										
					Case CONTROLdemo
									
						PLAYERai( p, 1, 5, GroundHeight )
						
					Case CONTROLmouse
									
						JX = (MouseXSpeed()/7.0)
						JZ = (MouseYSpeed()/7.0)
						Thrust = MouseDown(1) * F\Thrust 
						Shoot = MouseDown(2)
						MoveMouse HGW,HGH 
											
					Case CONTROLoriginal
					
						;change sensitivity
;						MouseSens = LIMIT (  MouseSens +  MouseZSpeed() * .025 , 0 , .8 )
						MouseSens = LIMIT (  MouseSens +  ( KeyDown(52) - KeyDown(51) ) * .025 , 0 , .8 )

						MouseMin = 50.0 * (1.0 - MouseSens )
						MouseMax = 250.0 * ( 1.0 - MouseSens )
						mx# = MouseX() - HGW
						mz# = MouseY() - HGH
						d# = Sqr( mx*mx + mz*mz )
						If d > MouseMax
							mx = mx / d * MouseMax
							mz = mz / d * MouseMax
							d = MouseMax
							MoveMouse HGW + mx, HGH + mz
						EndIf						

						;yaw
						RotateEntity MouseYaw, 0, p\yaw , 0
						If d > 0 
							PositionEntity MousePivot, mx, 0, - mz
							JX# = -DeltaYaw( MouseYaw , MousePivot )  * .2 / F\TurnSpeed
						Else
							JX# = 0
						EndIf	
		
						;pitch
						If d >= MouseMin
							JZ# =  -( ( ( d - MouseMin) / ( MouseMax - MouseMin ) *180.0 ) - p\pitch ) *.2 / F\TurnSpeed
						Else
							JZ# =  p\pitch * .2 / F\TurnSpeed
						EndIf
												
						Thrust = MouseDown(1) * F\Thrust 
						Shoot = MouseDown(2)
																				
					Case CONTROLjoypad
									
						JX=JoyX():If Abs(jx)<.25 JX = 0
						JZ=JoyY():If Abs(jz)<.25 JZ = 0
						Thrust = JoyDown(3) * F\Thrust 
						Shoot = JoyDown(8) 
																				
				End Select
				
				;no pitch on landing lad
				If p\y = 2.9 JZ = 0
				
				;actions
				
				Launch = ( KeyDown(57) Or JoyDown(6) And Missiles > 0 )  
				Infect = 0
				Bomb = 0
				Attract=0
				ShootAt = 0
				p\fuel = limit( p\fuel - Thrust*10.0, 0 , 500 )
				Score=Score-(Shoot And p\reload=0)
					
				If AlliesON
					If Thrust > 0 And AlliesGO = False 
						AlliesGO = True
						PlaySound ( SOUNDbabble )
					EndIf
				EndIf
												
			Else						
		
				PLAYERai( p, (p\index = 0) , 5, GroundHeight )
		
			EndIf			
			
			;allies don't start until you do!			
			If p\AI = 18
				If Not AlliesGO
					Thrust = 0
					Shoot=0
					JX = 0 
					JZ = 0
				EndIf
			EndIf
			
			;update player position
			If F\engine = 0 And ( p\y > MaxHeight  Or p\fuel = 0 ) Thrust = 0
			p\spin = wrap( p\spin+F\SpinSpeed, 360 ) 
			p\pitch =  limit( p\pitch - JZ * F\TurnSpeed , F\PitchMin, F\PitchMax )
			p\yaw = p\yaw - JX * F\TurnSpeed
			RotateEntity p\model , p\pitch , p\yaw , 0
			TFormNormal 0,(F\engine=0),(F\engine>0), p\model,0
			If F\SpinSpeed > 0 RotateEntity p\model,p\pitch+p\spin*( p\ai=3 Or p\ai=19 ),p\yaw+p\spin,0,1
			p\vx = p\vx * F\momentum + TFormedX() * Thrust
			p\vy = p\vy * F\momentum + TFormedY() * Thrust - ( ( F\engine=0 ) + ( p\y > MaxHeight ) ) * gravity
			p\vz = p\vz * F\momentum + TFormedZ() * Thrust
			p\x = wrap( p\x + p\vx, size)
			p\z = wrap( p\z - p\vz, size)
			p\y = p\y + p\vy
			
			;check ground collisions / refuel
			GroundHeight# = TERRAINgetheight( p\x , p\z, 1 ) + .5  * ( p\AI<>17 )
			If p\y < GroundHeight
				
				t.TerrainType = terrain( Floor(p\x), Floor(p\z) )
				p\y = GroundHeight
														
				If p\Index = 0
					Refuel = ( p\vy >-.1 And p\pitch < 6 And t\LandIndex = 3 And GroundHeight = 2.9 )
				Else
					Refuel = False 
				EndIf
				
				If Not Refuel
					;dead
					p\dead = 1
					COLLISIONground( p\x , p\y , p\z , p\InView , True, p\Index )
				Else
					;refuel
					p\Pitch = 0
					p\fuel = LIMIT( p\fuel + 1.5 , 0,500 )
					p\vy = -p\vy * .5
					p\vx = p\vx * .75
					p\vz = p\vz * .75
					p\pitch = 0
				EndIf
							
			EndIf
			
			;position in local coords
			PLAYERposition( p )
			
			;Actions
			If Attract And p\reload = 0 PLAYERattract( p, GroundHeight )
			If ShootAt And p\reload = 0 PLAYERshootat( p )
			If Launch And p\counter=0 PLAYERlaunch( p )
			
			;smoke if on last legs
			If F\engine <> 2 And p\fuel <= 100 And Rand(0,3)=0
				PARTICLEnew( Grid\Model,14, p\x , p\y, p\z,0, 0 , 0, 2)
			EndIf
									
			;particles
			If p\InView
			
				If Thrust > 0 And p\thrustcounter = 0
					p\thrustcounter = F\ThrustRate
					If F\engine = 0 PARTICLEnew( p\Model, 0, p\x,p\y,p\z, 0,-.5,0, 3, p\vx,p\vy,p\vz )
					If F\engine = 2 PARTICLEnew( p\Model, 2, p\x,p\y,p\z, 0,0,-1, 1 )
					If F\engine = 3 PARTICLEnew( p\Model, 18 ,p\x,p\y,p\z,0,0,-3.5,3, p\vx,p\vy,p\vz,p\index )
				EndIf
				
				If p\Reload = 0				
					If Infect   p\Reload = F\firerate: PARTICLEnew( p\Model, 11, p\x,p\y,p\z, 0,.5,0, 1 )
					If Bomb  p\Reload = F\firerate: PARTICLEnew( p\Model, 12, p\x,p\y,p\z, 0,-.5,0, 1 , p\vx,p\vy,p\vz, p\index)
				EndIf
				
			Else

				;infect landscape if seeder / bomber off screen
				If p\Reload=0 And ( p\AI = 1 Or p\AI = 2 )
					an = Rand( 0,360):ra = Rand(6)
					MAPadd( 0, p\x + ra * Cos(an), p\z + ra * Sin(an) )
				EndIf

			EndIf

			;Only fire bullets if in local space UNLESS there are Allied Craft
			If p\InView Or AlliesON
				If p\Reload = 0				
					If Shoot
						If p\InView
							CHANNELshoot = PlaySound( SOUNDshoot )
							ChannelPitch CHANNELshoot, 44000+p\Ai *1000 ;20000 + p\Ai * 1000
						EndIf
						p\Reload = F\firerate
						For ga=-p\spread To p\spread 
							If Not ShootAt 
								RotateEntity p\gun,0,ga*3,0 
								vx# = p\vx:vy#=p\vy:vz#=p\vz
							Else 
								vx#=0:vy#=0:vz#=0
							EndIf
							PARTICLEnew( p\gun, 1, p\x,p\y,p\z, 0,0,.25, 1 , vx , vy , vz , p\index )
						Next
					EndIf
				EndIf
			EndIf
			
			;radar dot
			If p\index = 1 And terrain(Floor(p\x), Floor(p\z))\LandHidden
				HideEntity p\dot
			Else
				ShowEntity p\dot
				mapx# = -246  + ( p\x -128.0 )  *.5
				mapy# = 166 + (128.0 - p\z ) * .5
				PositionEntity p\dot, mapx,mapy,0
			EndIf
			
			;3d engine sound details
			If Thrust > 0
				G.SoundGroupType = SoundGroup( F\SoundGroup )
				If DistanceLUP( cam , p\id ) < G\distance
					G\distance = DistanceLUP( cam , p\id )
					G\volume =  ( 3.0 + ( p\id = cam ) ) * .25
					G\id = p\id
				EndIf
			EndIf
		
		EndIf
		
		;handle explosions
		If p\dead > 1
			p\dead=p\dead+1
			If p\dead = F\deadtimer 
				p\dead=-1
				Enemies = Enemies - ( p\index = 1 )
			EndIf
		EndIf
		If p\dead = 1
			If p\Index = 0 And p\ID <> 0 PlaySound ( SOUNDbabble )
			CHANNELexplode = PlaySound ( SOUNDexplode )
			p\dead=2
			p\fuel=0
			If p\Id = cam ChannelVolume SoundGroup( F\SoundGroup )\channel, 0
			If p\id = 0 EntityParent HUD\crosshair,0:HideEntity HUD\crosshair
			p\vx=0:p\vy=0:p\vz=0
			EntityPickMode p\model, 0
			DebugLog p\Id
			HideEntity p\model
			HideEntity p\dot
			If p\InView
				PARTICLEnew( Grid\Model, 13 , p\x, p\y, p\z, 0, .5, 0, 40 * F\radius )
				PARTICLEnew( Grid\Model, 3 , p\x, p\y, p\z, 0, 1, 0, 10 )
			EndIf
		EndIf
			
	Next
	
	;player / player collisions
	COLLISIONplayer()
	
	;Play 3d Sounds
	For G.SoundGroupType = Each SoundGroupType
		If G\distance < 2500
			volume# = G\volume * ( 1.0 / ( G\distance*.004 + 1.0 ) ) 
		Else
			volume# = 0
		EndIf	
		ChannelVolume G\channel, volume
		G\distance = 2500
		G\volume = 0
	Next
	
	;Ambient Sound
	If SOUNDtimer = 0 
		CHANNELthunder = PlaySound ( SOUNDthunder )
		ChannelVolume CHANNELthunder, Rnd( .1,1 ) * ( player(cam)\y < 40 )
		SOUNDtimer = Rand(FPS*5, FPS*60 ) 
	Else
		SOUNDtimer = SOUNDtimer - 1
	EndIf
	ChannelVolume CHANNELwind, .5  - LIMIT( player(cam)\y , 0, 50 )/100.0  


End Function

;=================================================
;=================================================
;=================================================

Function PLAYERposition ( c.PlayerType )	

	lx#=c\x - player(cam)\x : If Abs( lx )>128  lx = (Size-Abs(lx))*Sgn(-lx)
	lz#=c\z - player(cam)\z : If Abs( lz )>128  lz = (Size-Abs(lz))*Sgn(-lz)
	View = Grid\Divs * 2
	c\InView = ( Abs(lx) < View And Abs(lz) < View )
	PositionEntity c\model,lx,c\y,-lz
	
End Function

;=================================================
;=================================================
;=================================================

Function PLAYERai( p.PlayerType, Index, AttackHeight#, GroundHeight# ) 
	
	F.FlyingObjectType = FlyingObject( p\ai )
	Thrust = F\Thrust
	Shoot = 0
	Launch = 0
	ShootAt = ( p\AI = 9 )
	Attract = ( p\AI = 8 )
	Infect = ( p\AI = 1 )
	Bomb = ( p\AI = 2 )
	CheckHeight# = GroundHeight + F\CruiseHeight
	
	Select F\Engine
	
	Case 0	
		
		If p\target > -1
		
			range# = F\range * F\range
			distance# = PLAYERdistance( p , player(p\target) )
			PLAYERchase( p , player(p\target) )
			If p\target > -1
				If Abs( DY ) > 30 And p\Index = 0 Thrust = 0
				Shoot = ( distance < (15*15) And Abs(DP) < 90 ) 
				If distance < range CheckHeight# = player(p\target)\y+AttackHeight
				MaxPitch# = 10.0 + ( (45.0 + 45.0*(p\index=0) ) * distance / range )
				If player(p\target)\dead<>0 Or ( DistanceLUP( p\id , p\target ) > range ) p\target=-1
				
				;boss
				If p\ai = 16 And distance < ( 20 * 20 ) launch = True
				
			EndIf
			
		Else
			MaxPitch# = Rand(10,45)
			JX = Rnd(-.25,.25 )
			PLAYERfindtarget( p, index, index, F\range )
		EndIf
		If Shoot=0 JZ=-Sgn( MaxPitch - p\pitch)
		If p\y >  ( CheckHeight  + AttackHeight*.5 ) Thrust = 0 
		If ( p\y <  CheckHeight) Or ( p\vy <-.25 )
			If p\y< MaxHeight JZ = -Sgn(  Rand(10,45) - p\pitch):Thrust = F\Thrust
		EndIf
		Clear# = p\y - GroundHeight
		If Clear <= 10
			JZ = - Sgn( Rand(5, 4.5*Clear)-p\pitch )
			Thrust = F\Thrust
		EndIf
		
						
	Case 1,3 
	
		p\y = p\y + ( CheckHeight - p\y ) * .025
		JX = Rnd(-.25,.25)
		JZ = 0
		;geneators / mystery / cruiser
		If p\ai > 9 And p\ai < 16
			PLAYERfindtarget(p,0, 2, F\range )
			If p\target > -1 launch=1
		EndIf
				
	Case 2  ;missile / spore / pest
		JX = 0
		JZ = 0
		;missile
		If p\AI = 20
			Thrust = Thrust * (1.0 + ( p\fuel=500 ) * 10.0 ) 
			p\fuel = p\fuel - 1
			If p\fuel < 0 p\dead=1
			If p\fuel < 490
				If p\target > -1  
					PLAYERchase( p , player(p\target) )
				Else
					PLAYERfindtarget( p , 1, 1 )
				EndIf 
			EndIf
		EndIf
		
		;spore  / pest
		If p\AI = 3 Or p\AI = 19
			If p\target > -1
				PLAYERchase( p, player(p\target) )
			Else
				PLAYERfindtarget( p, 0, 0 )
			EndIf
			If p\y < CheckHeight JZ = 2
			If p\y > Maxheight   JZ = -2
		EndIf
			
	End Select

End Function

;=================================================
;=================================================
;=================================================

Function PLAYERattract( p.PlayerType, GroundHeight# )

	PLAYERfindtarget( p , 0 , 2, 12 )
	Child = GetChild( p\model , 2 )
		
	If p\InView
	
		If p\target > -1
			;attract player
			T.PlayerType  = player( p\target ) 
			PositionEntity Temp1, EntityX( T\Model ), EntityY( T\Model ), EntityZ( T\Model )
			D# = Sqr( PLAYERdistance( p, T) )
			T\vx = T\vx - NX*.075
			T\vy = T\vy - Abs( NY*.075 )
			T\vz = T\vz - NZ * .075
			T\fuel = LIMIT( T\fuel - 2.0, 0, 500 )
			PARTICLEnew( Child,  15, T\x , T\y , T\z, 0,0,0, 1 )
		Else
			;destroy landscape
			PositionEntity Temp1, EntityX( p\Model ), GroundHeight, EntityZ( p\Model )
			D# = EntityDistance ( p\model, Temp1 )
			COLLISIONground( p\x, GroundHeight, p\z, True, False, 1 )
			PARTICLEnew( Grid\Model,  15 , p\x , GroundHeight , p\z, 0,0,0, 3 )
		EndIf
		
		;update beam
		ShowEntity Child
		cs=GetSurface( Child ,1 )
		ts=GetSurface( Tractor ,1 )
		For v=1 To 6
			If v=1 Or v=4 rx#=Rnd(-10,10):ry#=Rnd(-10,10)
			vx#=VertexX(ts,v)+rx
			vy#=VertexY(ts,v)+ry
			vz#=VertexZ(ts,v)
			VertexCoords cs,v,vx,vy,vz
		Next
		PointEntity Child , Temp1
		ScaleEntity Child , .125 , .125 , D 
				
	Else

		HideEntity Child

	EndIf

End Function

;=================================================
;=================================================
;=================================================

Function PLAYERshootat( p.PlayerType )

	PLAYERfindtarget ( p , 0 , 2 ,15 )
	If p\target > -1
		PointEntity p\gun,player(p\target)\model
		TurnEntity p\gun,Rand(-15,15),-15 + Rand(-15,15),0
		shoot=1
	EndIf

End Function

;=================================================
;=================================================
;=================================================

Function PLAYERlaunch( p.PlayerType )

	;find next available missile/child
	Found = -1
	For m =0 To 2
		Check = p\child[ m ]
		If player( check )\dead=-1 Found = Check
	Next
		
	;Launch It		
	If Found > -1	
		L.PlayerType = player( Found )  
		L\fuel = 500
		offset# = ( FlyingObject( p\ai )\radius + FlyingObject( L\ai)\radius*.25 )
		TFormNormal 0,.1,1,p\model,0
		L\vx = TFormedX() 
		L\vy = TFormedY()  
		L\vz = TFormedZ()  
		L\x = p\x + TFormedX()*offset
		L\y = p\y + TFormedY()*offset
		L\z = p\z - TFormedZ()*offset
		L\yaw = p\yaw + p\spin
		L\pitch = p\pitch
		PLAYERposition( L )
		p\counter = FlyingObject( p\ai )\missilerate
		L\counter = FlyingObject( L\ai)\missilerate
		Missiles = Missiles - (p\AI = 0 )
		L\dead=0
		EntityPickMode L\model, 1
		If p\InView CHANNELlaunch = PlaySound ( SOUNDlaunch )
		RotateEntity L\model, L\pitch , L\yaw,0
		ShowEntity L\model
		Enemies = Enemies + ( L\index=1 )
	EndIf
	
End Function

;=================================================
;=================================================
;=================================================

Function PLAYERchase( p.PlayerType , t.PlayerType )

	lx# = p\x-t\x:If Abs(lx)>128 lx=(size-Abs(lx))*Sgn(-lx)
	lz# = p\z-t\z:If Abs(lz)>128 lz=(size-Abs(lz))*Sgn(-lz)
	
	px# = EntityX( p\model) + lx
	pz# = EntityZ( p\model ) - lz
	PositionEntity Temp1 , px , t\y , pz
	DY# = DeltaYaw( p\model, Temp1 )
	DP# = DeltaPitch(p\model,Temp1 )
	JX = Sgn( DY ) 
	JZ = -Sgn( DP )
			
	p\ChaseTimer = p\ChaseTimer + 1
	If t\dead <> 0 Or p\ChaseTimer > ( FPS * 10 ) p\target = -1
		
End Function

;=================================================
;=================================================
;=================================================

Function PLAYERfindtarget( p.PlayerType , index1 , index2, range#=1000 )

	range = range * range
	p\target = -1
	
	If ( index1 + index2 )= 0 And ( Not AlliesON )
		If player(0)\dead = 0 And DistanceLUP( p\Id, 0 ) < range p\target = 0
	Else
		For t.PlayerType = Each PlayerType
			If t\dead = 0 And ( t\index = index1 Or t\index = index2 )
				distance# = DistanceLUP( p\id , t\id ) 
				If distance < range range=distance:p\target = t\id
			EndIf
		Next
	EndIf
	
	If p\target > -1 p\ChaseTimer = 0
	
End Function

;=================================================
;=================================================
;=================================================

Function PLAYERdistance#( p.PlayerType , t.Playertype )

	dx# = p\x-t\x:If Abs(dx)>128 dx=(size-Abs(dx))*Sgn(-dx)
	dz# = p\z-t\z:If Abs(dz)>128 dz=(size-Abs(dz))*Sgn(-dz)
	dy# = p\y-t\y
	d# = ( dx*dx + dy*dy + dz*dz ) + .01
	;normals for collisions
	NX = -dx / d
	NY = -dy / d
	NZ = dz / d
	Return d 

End Function

;=================================================
;=================================================
;=================================================

Function PLAYERdistanceinit()

	For o=0 To MaxPlayers
		For q=0 To MaxPlayers
			DistanceLUP(o,q) = PLAYERdistance( player(o) , player(q) )
		Next
	Next

End Function

;=================================================
;=================================================
;=================================================

Function SCOREtagadd( x#, y#, z#, Frame, Points )

	If points <> 0
	
		score = score + Points
		st.ScoreTagType = ScoreTag( ScoretagNext )
		st\x = x
		st\y = y + 1
		st\z = z
		st\life = 1.0
		EntityTexture st\sprite,scores, Frame , 0  
		ScoreTagNext = WRAP ( ScoreTagNext + 1 , MaxScoretags )
		
	EndIf

End Function

;=================================================
;=================================================
;=================================================

Function SCOREtagupdate()

	For st.ScoreTagType = Each ScoreTagType
		If st\life > 0
			st\life = st\life - .01	
			st\y = st\y + .01
			lx# = st\x - player(cam)\x : If Abs( lx )>128  lx = (Size-Abs(lx))*Sgn(-lx)
			lz# = st\z - player(cam)\z : If Abs( lz )>128  lz = (Size-Abs(lz))*Sgn(-lz)
			View = Grid\Divs * 2
			InView = ( Abs(lx) < View And Abs(lz) < View )
			PositionEntity st\sprite,lx,st\y,-lz
			EntityAlpha st\sprite,st\life
			PointEntity st\sprite, Camera
		EndIf	
	Next
	
End Function

;=================================================
;=================================================
;=================================================

Function GAMEinit()

	;scores
	Scores = LoadAnimTexture("MiscData\Scores.bmp", 48+4,64, 32, 0, 15)
	For l = 0 To MaxScoreTags
		ScoreTag(l) = New ScoreTagType
		ScoreTag(l)\sprite = MESHquad( 0, 0, 1, .5,0 )
		EntityAlpha ScoreTag(l)\sprite,0
		ScoreTag(l)\life = 0
	Next

	;camera
	camera= CreateCamera()
	CameraZoom camera,1.25
	CameraFogMode camera, 1

	;hud
	HUDinit()
	
	;sound
	SOUNDinit()

	;menus
	MENUinit()
	
	;particles
	PARTICLEinit()
		
	;shadows
	ShadowSize = 256 ;512 ;256
	Shadow = CreateCamera()
	CameraViewport Shadow,0,0,ShadowSize, ShadowSize
	PositionEntity Shadow,0,1000,0
	RotateEntity Shadow,90,0,0
	CameraProjMode Shadow,2
	CameraClsColor Shadow,255,255,255
	HideEntity Shadow
	ShadowTexture = CreateTexture( Shadowsize, ShadowSize,48+256)
	SetBuffer TextureBuffer(ShadowTexture)
	Color 255,255,255:Rect 0,0,ShadowSize,ShadowSize,1
	SetBuffer BackBuffer()
	TextureBlend ShadowTexture, 2
		
	;terrain
	For z = 0 To Size -1
		For x =0 To Size -1
			terrain( x, z ) = New TerrainType
			terrain(x, z)\wavefactor = Sqr( (x-128)^2 + ( z-128)^2 ) * 45.0
		Next
	Next
	
	;players
	For l = 0 To 100
		player(l) = New PlayerType
		player(l)\Dead = -1
	Next

	Restore GameObjectData

	;Ground Objects
	For l=1 To GroundObjects
		GroundObject( l )=New GroundObjectType
		G.GroundObjectType = GroundObject( l )
		Read name$
		;normal
		G\Model[0] = MESHload( name$,0,2+4,0,0)
		;infected 
		G\Model[1] = MESHchange(G\Model[0] , 0)
		;destroyed 
		G\Model[2] = MESHchange(G\Model[0] , 1+ ( l>4 And l<8 ) )
				
		If name$ = "Radar1"
			Radar = MESHload( "Radar2" , 0 , 2+4 , 0 , 0 )
			temp=CopyEntity( Radar , G\Model[0] )
			temp=MESHchange( Radar , 0 )
			EntityParent temp,G\Model[1]
			FreeEntity Radar
		EndIf
		If name$ = "Mill1"
			Mill = MESHload( "Mill2" , 0 , 2+4 , 0 , 0 )
			temp = CopyEntity( Mill , G\Model[0] )
			PositionEntity temp,0,20.0/16.0,0
			temp = MESHchange( Mill , 0 )
			EntityParent temp , G\Model[1]
			PositionEntity temp,0,20.0/16.0,0
			FreeEntity Mill
		EndIf
		G\height = MeshHeight(G\Model[0]) + .5 
	Next
	
	;Flying Objects
	Tractor = MESHload("Tractor",0,2+4,0,0,1 )
		
	For l = 0 To FlyingObjects
		FlyingObject( l ) = New FlyingObjectType
		F.FlyingObjectType = FlyingObject( l )
		Read F\Name$
		Read F\Points
		Read F\tag
		Read F\Thrust
		Read F\Momentum
		Read F\FireRate
		Read F\MissileRate
		Read F\Damage
		Read F\SpinSpeed
		Read F\engine
		Read F\PitchMin
		Read F\PitchMax
		Read F\TurnSpeed
		Read F\Range
		Read F\r
		Read F\g
		Read F\b
		Read F\CruiseHeight
		Read F\SoundGroup

		F\DeadTimer = 100 - 95*( l=20 )
		F\Model= MESHload( F\name$ ,0 , 2+4 ,  0 , 0 )
		Radius# = Sqr( MeshWidth(F\Model)^2 + MeshDepth( F\Model)^2 )
		F\Radius = LIMIT ( Radius*.4  , .625, 100 )
		F\ThrustRate = 5 * (l>0 And l<20 And l<>15 ) 
	
	Next
			
	FADE( 0,-1)
				
End Function

;=================================================
;=================================================
;=================================================

Function GAMEstart()

	FADE( 0, -.05 )
	HideEntity HUD\arrow
	HideEntity HUD\model
	HideEntity HUD\height
	HideEntity HUD\bonus
	HideEntity HUD\fuel
	ShowEntity HUD\title
	FADE( 2.0, .05 )
	MENUshow( MENU(0), True )
	FADE( 0,-.05 )
	ShowEntity  HUD\model
	ShowEntity HUD\arrow
	ShowEntity HUD\height
	ShowEntity HUD\bonus
	ShowEntity HUD\fuel
	HideEntity HUD\title
	
	CONTROLcurrent = menu(0)\sub_current[3]
	CAMERAcurrent = menu(0)\sub_current[4]
	Difficulty = menu(0)\sub_current[1]
	Lives = 4 - Difficulty 
	Missiles= 4 - Difficulty 
	Gravity = 0.003 + 0.002 * Float( menu(0)\sub_current[1] )
	Score=0
	LastWave = 0
	Wave = menu(0)\sub_current[2] * 4 
	BonusLife = 5000
	QUIT = 0
		
End Function

;=================================================
;=================================================
;=================================================

Function GAMEupdate()

	;crosshair targeting
	If CrossHairON 
		p.PlayerType = PLAYER( 0 )
		TFormVector 0,0,20,p\model,0
		unused = LinePick( EntityX(p\model),EntityY(p\model),EntityZ(p\model), TFormedX(),TFormedY(),TFormedZ()  )
		picked = PickedEntity()
		If picked > 0
			HUD\Ontarget = LIMIT( HUD\Ontarget + .05, 0, 1.0 )
			radius# = EntityName( picked ) 
			EntityParent HUD\crosshair, picked
			PositionEntity HUD\crosshair,0,0,0
		Else
			parent = GetParent( HUD\CrossHair )
			If parent > 0 radius# = EntityName( parent ) 
			HUD\OnTarget = LIMIT( HUD\OnTarget - .05 , 0,1.0 )
		EndIf
					
		If HUD\Ontarget = 0 
			HideEntity HUD\crosshair
		Else
			ShowEntity HUD\crosshair
			sc#  = ( 20.0 - 8.0 * HUD\Ontarget ) * radius
			PointEntity HUD\Crosshair, camera
			ScaleEntity HUD\CrossHair,sc,sc,sc
			EntityColor HUD\CrossHair,100*HUD\OnTarget,50*HUD\Ontarget,0
		EndIf
	EndIf
	
	;pause
	If KeyDown(25) 
		MENUshow( menu(3) , False )
		QUIT = ( menu(3)\current = 2 ) 
		If Not QUIT SOUNDon( True )
	EndIf
	
	;change camera target
	nw=KeyDown(60)-KeyDown(59)
	If KeyDown(61) cam = 0
	If nw<>0 
		Repeat
			cam = WRAP( cam+nw, MaxPlayers ) 
		Until player(cam)\dead=0
		CAMERAupdate( 1 )
		Delay 500
	EndIf
		
	;change grid size
	LastSize# = GridSize
	GridSize = LIMIT ( GridSize + .25*( KeyDown(68)-KeyDown(67) ) , 0, 3 )

	;change camera type	
	If KeyDown(87) 
		CAMERAcurrent = WRAP( CAMERAcurrent + 1, 3 )
		CAMERAupdate( 1 )
		LastSize = -1
		Delay 500
	EndIf
	
	;change grid size / also if camera changes for fog range
	If LastSize<>GridSize MESHgrid( GridSize )
	
	;change control type
	If KeyDown(66) 
		CONTROLcurrent = WRAP ( CONTROLcurrent + 1, 4 )
		player(cam)\target = -1
		Delay 500
	EndIf
		
	;bonus timer
	If BonusTimer > 0 
		BonusTimer = LIMIT( BonusTimer - 1, 0, FPS*30 )
		If BonusTimer = 0 player(0)\spread = 0
	EndIf
	
	;Bonus Life / Missile
	If Score >= BonusLife 
		Lives = Lives + 1
		Missiles = Missiles + 1 
		BonusLife = BonusLife + 5000
	EndIf

	;Dead
	If p\dead=-1 
		Lives = Lives -1
		If Lives > 0 
			FADE(0,-.025)
			cam=0
			BonusTimer=0
			PLAYERreset ( 0, 0 ) 
			PLAYERdistanceinit()
			PLAYERupdate()
			FADE(1,.025)
		Else 
			GAMEOVER = True 
		EndIf
	EndIf
		
	;fireworks
	If Wave = 22 
		If ( Enemies Mod 90 ) = 0 CHANNELsplat = PlaySound ( SOUNDfirework )
		Enemies = Enemies - 1
		If Enemies = 449 PlaySound ( SOUNDfanfare )
		angle# = Rand(-180,180)
		rx# = 3.0*Cos(angle)
		rz# = 3.0*Sin(angle)
		PARTICLEnew( Grid\Model, 19 , 128, 2.9, 128, rx,0,rz, 1 )
	EndIf
	
	WAVECOMPLETE = ( Enemies = 0 )
	
	;cheats
	If KeyDown(64) lives = lives + 1: Missiles = Missiles + 1
	If KeyDown(62) WAVECOMPLETE = True
	
	HUDupdate()

End Function

;=================================================
;=================================================
;=================================================

Function GAMEend()

	If Score > BestScore BestScore = Score
	HUDupdate()
	If Wave < 22 
		MENUshow ( menu(2), False )
	Else
		MENUshow( menu(4),False )
	EndIf

End Function

;=================================================
;=================================================
;=================================================

Function MAPadd( mode, sx, sz , ss=1 )

	Select mode
	
	Case 0	;infected

		For vx = -ss To ss
			x = wrap( sx + vx , size)
			For vz = -ss To ss
				z = wrap( sz + vz , size)
				t.TerrainType = terrain(x,z)
				If t\LandInfected = 0  
					t\r[0] = t\r[1]  
					t\g[0] = t\g[1]
					t\b[0] = t\b[1] 
					t\LandInfected = 1
					AreaInfected = AreaInfected + 1
					If t\ObjectStatus = 0 t\ObjectStatus = 1
					If t\LandHidden = 0
						MAPchange( 0, MAPcounter ) = x
						MAPchange( 1, MAPcounter ) = z
						MAPchange( 2, MAPcounter ) = t\argb[1]
						MAPcounter = MAPcounter + 1
					EndIf
				EndIf
			Next
		Next
		
	Case 1	;hidden
	
		sx = Floor( ( sx+16 ) / 32 ) * 32
		sz = Floor( ( sz+16 ) / 32 ) * 32 
		For vx = -16 To 15
			x = wrap( sx + vx , size )
			For vz = -16 To 15
				z = wrap( sz + vz , size )
				terrain(x,z)\LandHidden = 1
				MAPchange( 0, MAPcounter ) = x
				MAPchange( 1, MAPcounter ) = z
				MAPchange( 2, MAPcounter ) = 0
				MAPcounter = MAPcounter + 1
			Next
		Next

	End Select

End Function

;=================================================
;=================================================
;=================================================

Function MAPupdate()

	Buffer = TextureBuffer( HUD\map )
	SetBuffer Buffer
	LockBuffer Buffer
	For l= 1 To MAPcounter
		WritePixelFast MAPchange( 0, l ), MAPchange( 1, l), MAPchange( 2, l )
	Next 
	UnlockBuffer Buffer
	SetBuffer BackBuffer()
	MAPcounter = 0

End Function

;=================================================
;=================================================
;=================================================

Function TERRAINupdate()

	p.PlayerType = PLAYER( cam )

	;delete objects from previous frame
	FreeEntity pivot:pivot=CreatePivot()
	
	ax# = Floor( p\x )
	az# = Floor( p\z )
	fx# = p\x  - ax
	fz# = p\z - az  
	
	If p\y > 25
		;star particles
		For l = 0 To GridSize*2+2
			rx = Rand( -Grid\View, Grid\View ) + p\vx * 20.0
			rz = Rand(-Grid\View ,Grid\View ) - p\vz * 20.0
			ry = LIMIT( p\y + Rand( -Grid\Divg, Grid\Divg ), 25, 400 )
			PARTICLEnew ( Grid\Model, 16 + ( ry > MaxHeight ) , p\x, 0, p\z , rx, ry, -rz, 1 )
		Next
	Else
		;rain particles
		No = ( 1+ GridSize * 2.0 ) * ( ( Rand( AreaTotal ) -50 ) < AreaInfected * 4.0 )
		
		If No > 0
			For l = 1 To No
				rx = Rand(-Grid\Divs , Grid\Divs ) + p\vx*FPS 
				rz = Rand(-Grid\Divs , Grid\Divs ) - p\vz*FPS 
				ry = LIMIT( p\y + Rand( Grid\Divs , Grid\Divs+5 ) , 10,25 ) 
				PARTICLEnew ( Grid\Model, 9, p\x , 0 , p\z, rx,ry,-rz,1 )
			Next
		EndIf
	EndIf
		
	Ripple = WRAP ( Ripple + 15, 360 )
	Rotate = WRAP ( Rotate + 2 ,360 )
						
	For z = -Grid\Divs To Grid\Divs +1
		For x = -Grid\Divs To Grid\Divs+1
		
			;grid positions
			nx = wrap( ax + x , size ) 
			nz = wrap( az + z , size ) 
			t.TerrainType = Terrain( nx , nz )

			;vertex positions
			ox# = x - fx
			oz# = z - fz
			oy# = t\LandHeight
			vy# = t\LandHeight
			vx# = limit( ox , -Grid\Cull , Grid\Cull )
			vz# = limit( oz , -Grid\Cull ,Grid\Cull )

			;shadow texture coords
			tu# =.5 + ( vx / Float( Grid\Cull ) ) * .5  + (.5 / size)
			tv# =.5 + ( vz / Float( Grid\Cull ) ) * .5  + (.5 / size)
			
			;smooth edges
			If Grid\Cull <= 12
				If Abs( vx ) = Grid\Cull Or Abs( vz )=Grid\Cull Then vy#=TERRAINgetheight( p\x + vx , p\z + vz )
			EndIf
						
			;water waves
			If Abs( vy ) < .25 
				vy=vy+( .25-vy) * ( 1.0 + Cos( Ripple + t\WaveFactor ) )
			EndIf
			
			;cull soil
			If x = -Grid\Divs Or x = Grid\Divs+1 Or z = -Grid\Divs Or z = Grid\Divs+1 vy = 0
			
			;vertex positions	
			x1 = ( x + Grid\Divs ) * 2 - ( x > -Grid\Divs )
			z1 = ( z + Grid\Divs ) * 2 - ( z > -Grid\Divs )
			For iz = 0 To ( z > -Grid\Divs And z < Grid\Divs+1 )
				For ix = 0 To ( x > -Grid\Divs And x < Grid\Divs+1 )
					x2 = x1 + ix
					z2 = z1 + iz    
					v=x2+z2*(Grid\Divg+1)
					VertexCoords Grid\s,v, vx  , vy , -vz 
					VertexTexCoords Grid\s , v , tu , tv
				Next
			Next
			
			;vertex colours
			If x> -Grid\Divs And x < Grid\Divs And z> -Grid\Divs And z < Grid\Divs
				x1 = ( x + Grid\Divs ) * 2 
				z1 = ( z + Grid\Divs ) * 2 	
				For iz = 0 To 1 
					z2 = z1 + iz
					For ix = 0 To 1
						x2 = x1 + ix
						v = x2 + z2 * ( Grid\Divg+1 )
						VertexColor Grid\s,v,t\r[0],t\g[0],t\b[0] 
					Next
				Next
			EndIf
			
			;draw objects 
			If t\ObjectIndex > 0 
				If ( Abs( ox )- Float( Grid\Cull ) ) <.5 And ( Abs( oz )- Float( Grid\Cull ) ) < .5   
					temp = CopyEntity( GroundObject( t\Objectindex )\Model[ t\ObjectStatus] , Pivot)
					t\ObjectCycle= WRAP ( t\ObjectCycle+1, 20 ) 
					Tree = ( t\ObjectIndex < 5 )
					;objects normal
					If t\ObjectStatus < 2
						;radar					
						If t\ObjectIndex = 8 
							child = GetChild(temp,1)
							RotateEntity child, 0, (nx+nz)*10+rotate,0
							;If t\ObjectCycle = 0 
							;If Abs(DeltaYaw( child, player(cam)\model )) < 30 PARTICLEnew( child, 21, nx,oy+2,nz,0,0,1,1 )
						EndIf
							
						;mill
						If t\ObjectIndex = 9 RotateEntity GetChild(temp,1),0,0,rotate*5
						;crate
						If t\ObjectIndex = 11 t\ObjectYaw = rotate*2
						;tree sway
						If Tree
							t\ObjectPitch = t\ObjectPitch+t\ObjectSway
							If Abs(t\ObjectPitch)>15 t\ObjectSway = - t\ObjectSway				
						EndIf
					EndIf
					RotateEntity temp, t\ObjectPitch , t\ObjectYaw , 0
					PositionEntity temp, ox, oy, -oz
					
					If t\ObjectCycle = 0
						;destroyed
						If t\ObjectStatus=2 PARTICLEnew( Grid\Model,3, nx , oy+GroundObject( t\ObjectIndex )\height*.75, nz,0, 0 , 0, 1)
						;infected
						If t\ObjectStatus = 1 And Tree PARTICLEnew( Grid\Model,10, nx , t\ObjectHeight, nz,0, .25, 0, 1)
					EndIf
					
				EndIf
			EndIf
			
		Next
	Next
	
	;UpdateNormals Grid\Model
		
	;make shadows
	ShowEntity shadow
	HideEntity camera
	HideEntity HUD\camera
	HideEntity Grid\Model
	AmbientLight 0,0,0
	RenderWorld()
	CopyRect 1,1,ShadowSize-2,ShadowSize-2,1,1,BackBuffer(),TextureBuffer(shadowtexture) 
	HideEntity shadow
	ShowEntity camera
	ShowEntity HUD\camera
	ShowEntity Grid\Model
	AmbientLight 255,255,255 ;160,160,160
	
End Function

;=================================================
;=================================================
;=================================================

Function TERRAINload( file$ )

	Map = LoadImage( "LevelData\"+file$ )
	SetBuffer ImageBuffer( Map )
	
	AreaTotal = 0
	AreaInfected = 0
	MAPcounter = 0
	
	;get heights
	For z =0 To Size -1
		For x =0 To Size -1
			GetColor x, z
			t.TerrainType = terrain(x,z)
			t\LandHeight = ColorGreen() * .05
			t\ObjectHeight = t\LandHeight 
			t\Landindex = ( ColorBlue() + 20 ) / 50 
			t\ObjectIndex = ( ColorRed() + 10 ) / 20
			t\ObjectYaw = Rand( 360 )
			t\ObjectPitch=0
			t\ObjectSway = 0
			t\ObjectStatus = 0
			t\ObjectCycle = ( x+z) Mod 20
			t\LandInfected = 0
			t\LandHidden = 0
			If t\ObjectIndex > 0 t\ObjectHeight = t\LandHeight + GroundObject( t\ObjectIndex )\height
	
			;trees
			If t\ObjectIndex < 5
				t\ObjectPitch = Rnd(-10,10)
				t\ObjectSway = ( Rand(0,1)*2-1)*.5
			EndIf
			;rockets
			If t\ObjectIndex = 10
				t\ObjectYaw = 0
			EndIf
		Next
	Next
	FreeImage Map
	
	;color terrain
	For z = 0 To Size -1
		For x =0 To Size -1
				
			th# = TERRAINgetheight( x+.5 , z+.5 ) * 20.0
			t.TerrainType = terrain(x,z )		
			Select t\LandIndex
			
			Case 0
				;sea
				t\r[0]=60+Rnd(40)
				t\g[0]=60+Rnd(40)
				t\b[0]=150+th*10+Rnd(40)
				t\LandInfected = 1
			Case 1
				;beach
				t\r[0]=th *10 + 60 
				t\g[0]=th *10 + 60 
				t\b[0]=200-th *10+Rnd(40)
			Case 2
				;land
				t\r[0]= th+Rand(60)
				t\g[0] = limit( th+60+Rand(60), 0, 255 )
				t\b[0] = th+Rand(60)
			Case 3
				;landing pad
				detail = ( ( z>125 And z< 131 ) And ( x = 125 Or x = 129) ) Or ( x>125 And x<129 And z=128 )
				col= ( detail * 155 ) + (  ( Not detail ) * Rand(100) ) + 100
				t\r[0]=col
				t\g[0]=col
				t\b[0]=col
			Case 4
				;alien buildings
				t\r[0] = LIMIT ( th + 128 , 0, 250 )
				t\g[0] = LIMIT( th+ Rand(32, 128) , 0, 250)
				t\b[0]= LIMIT ( th + 128, 0, 250 )
				t\LandInfected = 1
				
			End Select
			
			If t\LandIndex > 0 
				t\r[1] = ( t\r[0] +250 )* .4
				t\g[1] = ( t\g[0] + 50 ) *.4
				t\b[1] = ( t\b[0] + 100 ) *.4
			Else
				t\r[1] = t\r[0]
				t\g[1] = t\g[0]
				t\b[1] = t\b[0]
			EndIf
			
			;halfbright versions for map
			t\argb[0] = ( $FF Shl 24 Or t\r[0] Shl 16 Or t\g[0] Shl 8 Or t\b[0] And $FEFEFE ) Shr 1
			t\argb[1] = ( $FF Shl 24 Or t\r[1] Shl 16 Or t\g[1] Shl 8 Or t\b[1] And $FEFEFE ) Shr 1
			
		Next
	Next	
		
	;hud
	Buffer = TextureBuffer( HUD\Map )
	SetBuffer Buffer
	LockBuffer Buffer
	For z = 0 To Size-1
		For x= 0 To Size-1
			AreaTotal = AreaTotal + ( terrain(x,z)\LandIndex > 0 And terrain(x,z)\LandIndex < 4 )
			WritePixelFast x, z, terrain(x,z)\argb[0] 
		Next
	Next
	UnlockBuffer Buffer
	SetBuffer BackBuffer()
		
	;grid
	MESHgrid( GridSize )

End Function

;=================================================
;=================================================
;=================================================

Function TERRAINgetheight#( bx#,bz# , check=0 )

	bx=wrap( bx , Size)
	bz=wrap( bz , Size)
	tx#=Floor( bx )
	tz#=Floor( bz )
	ix#=bx - tx
	iz#=bz - tz  
	cx#=WRAP( tx+1.0,Size) 
	cz#=WRAP( tz+1.0,Size) 
		
	Select check
		Case 0
			v1#=terrain(tx,tz)\landheight+(terrain(cx,tz)\landheight-terrain(tx,tz)\landheight)*ix
			v2#=terrain(tx,cz)\landheight+(terrain(cx,cz)\landheight-terrain(tx,cz)\landheight)*ix
		Case 1
			v1#=terrain(tx,tz)\ObjectHeight + ( terrain(cx,tz)\ObjectHeight - terrain(tx,tz)\ObjectHeight )*ix
			v2#=terrain(tx,cz)\ObjectHeight + ( terrain(cx,cz)\ObjectHeight-terrain(tx,cz)\ObjectHeight)*ix
	End Select
		
	Return v1 + ( v2 - v1 ) * iz

End Function

;=================================================
;=================================================
;=================================================

Function CAMERAupdate( smooth#=.1 )

	p.PlayerType = PLAYER( cam )

	;move camera
	z# = LIMIT( 5 + Grid\Divs , 0, 32 )

	h# = 0 + GridSize

	Select CAMERAcurrent
		Case CAMERAoriginal
			PositionEntity camera, 0, limit( p\y, 5, 2000) + h, -z
			RotateEntity camera, 10, 0, 0
		Case CAMERAchase
			PositionEntity chase,0,0,0  
			RotateEntity chase,0, p\yaw , 0
			MoveEntity chase,0,0,-z
			cx# = EntityX( camera) + ( EntityX(chase) - EntityX(camera) ) * smooth 
			cy# = limit( p\y, 6, 2000 ) + h
			cz# = EntityZ( camera) + ( EntityZ(chase) - EntityZ(camera) ) * smooth 
			PositionEntity camera, cx, cy, cz
			RotateEntity camera, 10, EntityYaw(camera) +DeltaYaw(camera,p\model),0
		Case CAMERArear
			PositionEntity chase,0,0,0  
			RotateEntity chase,0, p\yaw , 0
			MoveEntity chase,0,0,-12
			cx# = EntityX( camera) + ( EntityX(chase) - EntityX(camera) ) * smooth 
			cy# = limit( p\y, 5, 2000 ) + h
			cz# = EntityZ( camera) + ( EntityZ(chase) - EntityZ(camera) ) * smooth  
			PositionEntity camera, cx, cy, cz
			RotateEntity camera, 10, EntityYaw(camera) +DeltaYaw(camera,p\model),0
			
	End Select
	
	;get camera yaw for particles facing camera
	CameraYaw = 360 + EntityYaw( camera ) 
	
End Function

;=================================================
;=================================================
;=================================================

Function WRAP#( q# , hi# , lo#=0 )
	
	If q < lo Then q = hi + (q-lo)
	If q >= hi Then q = lo + (q-hi)
	Return q
	
End Function

;=================================================
;=================================================
;=================================================

Function LIMIT#(q#,lo#,hi#)

	If q < lo q = lo
	If q > hi q = hi
	Return q
	
End Function

;=================================================
;=================================================
;=================================================

Function FADE( target#, speed#=.05 )

	If target = 1.0
		PLAYERupdate()
		HUDupdate()
		CAMERAupdate( 1 )
		PARTICLEupdate()
		MAPupdate()
		TERRAINupdate()
	EndIf
	
	If target = 2.0 target=1.0

	Repeat
		WaitTimer Timer
		FadeStatus = LIMIT ( FadeStatus + speed#, 0, 1.0 )
		EntityAlpha HUD\fade, (1.0 - FadeStatus )
		RenderWorld()
		Flip
	Until FadeStatus = Target

End Function

;=================================================
;=================================================
;=================================================

Function MESHgrid( Size# = 0 )

	Grid\cull = (2 ^ Size ) * 6
	Grid\divs = Grid\cull + 1
	Grid\divg = ( Grid\divs*2 ) * 2 + 1
	Grid\view = Grid\divs + 6
	Fade = Grid\View
		
	FreeEntity Grid\Model
	Grid\Model = CreateMesh()
	EntityFX Grid\Model,2
	s = CreateSurface( Grid\Model )
	For z=0 To Grid\divg
		For x=0 To Grid\divg
			v = AddVertex(s,0,0,0) 
			If x = 0 Or x = Grid\divg Or z = 0 Or z = Grid\Divg VertexColor s,v,0,0,0 Else VertexColor s, v, 80,50,30 
		Next
	Next
		
	For z=0 To Grid\divg-1 Step 2
		For x=0 To Grid\divg-1 Step 2
			v0 = x + z * ( Grid\divg+1 ) * ( z < ( Grid\divg + 1 ) )
			v1 = v0+1
			v2 = v1 + ( Grid\divg + 1 )
			v3 = v0 + ( Grid\divg + 1 )
			AddTriangle s,v0,v1,v2
			AddTriangle s,v2,v3,v0
		Next
	Next
	Grid\s = s
	
	;add something high up so ground always rendered
	temp = CreateSphere(3)
	s = GetSurface( temp, 1)
	For v = 0 To CountVertices( s )-1
		VertexColor s,v,0,0,0 
	Next
	PositionMesh temp,0,Maxheight + 100 ,0
	AddMesh temp, Grid\model
	FreeEntity temp
	
	;camera
	Select CAMERAcurrent
	Case CAMERAoriginal, CAMERAchase
		r# = LIMIT( 21+GridSize*2, 0, 46 ) 
		CameraFogRange camera,r, r+Fade ;+Grid\divs,r+Fade
		CameraRange camera, .1, r+Fade+10
	Case CAMERArear
		CameraFogRange camera,20+Grid\divs,20+Fade
		CameraRange camera, .1, 20+Fade+10
	End Select
		
	;shadow
	EntityTexture Grid\Model, ShadowTexture
	CameraZoom Shadow, ( .1668 / ( 2^size ) )
				
End Function	

;=================================================
;=================================================
;=================================================

Function MESHquad ( parent=0 , mode = 0, sx# = .75, sy# = .75, sz# = .75 )

	mesh = CreateMesh( parent )
	s = CreateSurface(mesh)
	
	v0 = AddVertex( s, -sx,sy,-sz,1,0)
	v1 = AddVertex( s, sx,sy,-sz,0,0)
	v2 = AddVertex( s, sx,-sy,-sz,0,1)
	v3 = AddVertex( s, -sx,-sy,-sz,1,1)

	If mode = 0
		AddTriangle s,v0,v3,v2
		AddTriangle s,v2,v1,v0
	EndIf
	
	If mode = 1
		AddTriangle s,v0,v1,v2
		AddTriangle s,v2,v3,v0
		v4 = AddVertex( s, -sx,-sy,sz)
		v5 = AddVertex( s, sx,-sy,sz)
		AddTriangle s,v0,v4,v5
		AddTriangle s,v5,v1,v0
	EndIf
	
	Return mesh
	
End Function

;=================================================
;=================================================
;=================================================

Function MESHload ( FileName$ , blend ,fx , parent , show , scale# = Oscale )

	FileName = "MeshData\"+FileName+".my3d"
	File=ReadFile( FileName )
	If File=0 RuntimeError FileName+": MY3D not found !!"
	
	DebugLog FileName
	
	mesh=CreateMesh():surfaces=ReadInt(file)
	
	For surface_num=1 To surfaces
					
		temp_surface=CreateSurface(mesh) 
		vertices=ReadInt(file)
		For vertex_num=0 To vertices-1
			x#=ReadFloat(file):y#=ReadFloat(file):z#=ReadFloat(file)
			temp_vertex=AddVertex(temp_surface,x,y,z)
			nx#=ReadFloat(file):ny#=ReadFloat(file):nz#=ReadFloat(file)
			VertexNormal temp_surface,temp_vertex,nx,ny,nz
			r#=ReadFloat(file):g#=ReadFloat(file):b#=ReadFloat(file)
			;r = r *.5:g=g*.5:b=b*.5
			VertexColor temp_surface,temp_vertex,r,g,b
			u#=ReadFloat(file):v#=ReadFloat(file):w#=ReadFloat(file)
			VertexTexCoords temp_surface,temp_vertex,u,v,w
		Next
		
		triangles=ReadInt(file)
		For triangle_num=0 To triangles-1
			v0=ReadInt(file):v1=ReadInt(file):v2=ReadInt(file)
			AddTriangle(temp_surface,v0,v1,v2)
		Next
	Next	
	
	PosX# = ReadFloat(file)
	PosY# = ReadFloat(file)
	
	CloseFile file
	
	EntityParent mesh, parent
	PositionEntity mesh , PosX , PosY , 0
	
	EntityBlend mesh,blend
	EntityFX mesh,fx
	;EntityShininess mesh,1
	
	;If scale = oscale EntityFX mesh, fx+4
	
	If Not show HideEntity mesh
	ScaleMesh mesh,scale,scale,scale
	
	MESHnormals( Mesh ) 
	;UpdateNormals Mesh
		
	Return mesh
End Function	

;=================================================
;=================================================
;=================================================

Function MESHchange( mesh , state=0 )
	
	nm=CopyMesh(mesh)
	s = GetSurface(mesh,1)
	ns = GetSurface(nm,1)
	
	For v=0 To CountVertices(s)-1
	
		If state=0
			r#=(VertexRed(s,v)+250)*.5 
			g#=(VertexGreen(s,v)+100)*.5 
			b#=(VertexBlue(s,v)+100)*.5
		Else
			r#=VertexRed(s,v)/255 * 80
			g#=VertexGreen(s,v)/255 * 50
			b#=VertexBlue(s,v)/255 * 30
		EndIf
		
		VertexColor ns,v,r,g,b
	Next
	
	If state=1 ScaleMesh nm,.5,.75,.5
	If state=2 ScaleMesh nm,1,.5,1
	
	EntityFX nm,2+4
	HideEntity nm
	Return nm		

End Function

;=================================================
;=================================================
;=================================================

Function MESHnormals( mesh )

	For l = 1 To CountSurfaces(mesh )
		s = GetSurface( mesh , l )
		;calculate normals for flatshading
		For t = 0 To CountTriangles( s )-1
			v0 = TriangleVertex( s, t, 0 )
			v1 = TriangleVertex( s, t, 1 )
			v2 = TriangleVertex( s, t, 2 )
			ax# = VertexX( s, v1 ) - VertexX( s, v0 )
			ay# = VertexY( s, v1 ) - VertexY( s, v0 )	
			az# = VertexZ( s, v1 ) - VertexZ( s, v0 )	
			bx# = VertexX( s, v2 ) - VertexX( s, v1 )
			by# = VertexY( s, v2 ) - VertexY( s, v1 )	
			bz# = VertexZ( s, v2 ) - VertexZ( s, v1 )	
			Nx# = ( ay * bz ) - ( az * by )
			Ny# = ( az * bx ) - ( ax * bz )
			Nz# = ( ax * by ) - ( ay * bx )
			Ns# = Sqr( Nx * Nx + Ny*Ny + Nz*Nz )
			Nx = LIMIT( Nx / Ns, -1,1 )
			Ny = LIMIT( Ny / Ns, -1, 1 )
			Nz = LIMIT( Nz / Ns, -1, 1 )
						
			For v = v0 To v2
				VertexNormal s, v, Nx, Ny, Nz 
			Next
		Next
	Next

End Function

;=================================================
;=================================================
;=================================================

Function PARTICLEupdate()

	s = GetSurface( ParticleMesh , 1 )
	ActiveParticles = 0
	SPLASHtimer = SPLASHtimer - ( SPLASHtimer > 0 )
		
	For p.ParticleType = Each ParticleType
					
		If p\Life >= 0
		
			ActiveParticles = ActiveParticles + 1
			;movement
			pt.ParticleTemplateType = ParticleTemplate ( p\Id )
			p\Life = LIMIT( p\Life - pt\fade ,0 , 1.0 ) 
			p\vy = p\vy - Gravity * pt\weight
			p\x = WRAP( p\x + p\vx, size )
			p\y = p\y + p\vy
			p\z = WRAP( p\z + p\vz, size )
			
			;local position
			lx# = ( p\x - player(cam)\x ):If Abs(lx)>128 lx=(size-Abs(lx))*Sgn(-lx)
			lz# = ( p\z - player(cam)\z ):If Abs(lz)>128 lz=(size-Abs(lz))*Sgn(-lz)
			
			;hazard particles
			If p\id = 1 Or p\id=12 Or p\id = 18 COLLISIONbullet( p )
			
			;fireworks
			If p\id = 19 And p\life = 0
				p\life = -1
				PARTICLEnew( Grid\Model,  20 , p\x,p\y,p\z, 0,0,0, 5 )
				PARTICLEnew( Grid\Model, 3 , p\x, p\y, p\z, 0, 1, 0, 10 )
			EndIf
				
			;ground collision 
			Groundheight# = TERRAINgetheight( p\x , p\z , 1 ) + ( p\size * OScale )
			If p\y < GroundHeight
				p\y = GroundHeight
				p\vx = p\vx * .5
				p\vy = -p\vy *.5
				p\vz = p\vz * .5
				Divs = Grid\Divs + 6
				InView =  Abs(lx) < Divs And Abs(lz) < Divs
				If p\Id = 1 COLLISIONground( p\x , p\y , p\z , InView, False, p\Index )
				If p\id = 10 Or p\id=11 MAPadd( 0 , p\x, p\z, p\id-9 )
				If pt\Splash > 0  
					p\Life = -1
					If InView
						cx# = wrap( Int( p\x ) , size )
						cz# = wrap( Int( p\z ) , size )
						If p\Id <> 12 Splash = 4 + Terrain(cx,cz)\LandIndex Else Splash = 11
						If p\Id < 2 And Splash < 6 And SPLASHtimer = 0
							CHANNELsplash = PlaySound( SOUNDsplash )
							ChannelPitch CHANNELsplash, Rand(10, 44 ) *1000
							SPLASHtimer = 5
						EndIf
						PARTICLEnew( Grid\Model,  Splash , p\x,p\y,p\z, 0,0,0, pt\Splash )
					EndIf
				EndIf
			EndIf
								
			Select Floor( pt\Alpha )
				Case 0	alpha# = pt\Alpha * p\Life
				Case 1	alpha# = Sgn( p\Life )
				Case 2	alpha# = 1.0 - Abs( p\life - .5 ) * 2.0
			End Select
				
			;vertices 
			For v=0 To 5
				px# = lx + ParticleLUP( 0, v, CameraYaw ) * p\size
				py# = p\y + ParticleLUP( 1, v, CameraYaw ) * p\size
				pz# = -lz + ParticleLUP( 2, v, CameraYaw ) * p\size
				VertexCoords s , p\Vertex + v , px, py, pz 
				VertexColor s, p\Vertex + v , p\r , p\g , p\b , alpha
			Next
			
			If p\life = 0 p\life = -1
			
		EndIf

	Next

End Function

;=================================================
;=================================================
;=================================================

Function PARTICLEnew( Mesh, Template, x#,y#,z#, ox#,oy#,oz#, Number, vx#=0, vy#=0,vz#=0, index=0 )

	pt.ParticleTemplateType = ParticleTemplate( Template )

	For l = 1 To Number
		p.ParticleType = particle( ParticleNext )
		TFormVector ox,oy,oz, Mesh, 0 
		p\x = x + TFormedX() - vx
		p\y = y + TFormedY() - vy
		p\z = z - TFormedZ() + vz
		TFormVector Rnd( pt\vx1,pt\vx2),Rnd(pt\vy1,pt\vy2),Rnd(pt\vz1,pt\vz2), Mesh, 0
		p\vx = TFormedX() + vx
		p\vy = TFormedY() + vy
		p\vz = -TFormedZ()  - vz
		p\r = Rand( pt\r1 , pt\r2) 
		p\g = Rand( pt\g1 , pt\g2) 
		p\b = Rand( pt\b1 , pt\b2)
		p\size = Rnd( pt\s1 , pt\s2)
		p\Life = 1.0
		p\Index = index
		p\Id = Template
		ParticleNext = WRAP ( ParticleNext + 1, MaxParticles )
	Next

End Function 

;=================================================
;=================================================
;=================================================

Function PARTICLEinit()

	;templates
	Restore ParticleTemplateData
	For l= 0 To ParticleTemplates
		ParticleTemplate(l) = New ParticleTemplateType
		pt.ParticleTemplateType = ParticleTemplate(l) 
		Read pt\splash
		Read pt\alpha
		Read pt\r1,pt\r2
		Read pt\g1,pt\g2
		Read pt\b1,pt\b2
		Read pt\s1, pt\s2
		Read pt\vx1, pt\vx2
		Read pt\vy1, pt\vy2
		Read pt\vz1, pt\vz2
		Read pt\weight
		Read pt\fade
	Next		
	
	;partcle LUP
	ParticleTemplateMesh = MESHquad( 0,1, OScale * .75, Oscale *.75, Oscale * .75 )
	S = GetSurface( ParticleTemplateMesh , 1)
	For a = 0 To 720
		RotateMesh ParticleTemplateMesh,0, 1 , 0
		For v = 0 To 5
			ParticleLUP( 0, v, a ) = VertexX( s, v ) 
			ParticleLUP( 1, v, a ) = VertexY( s, v ) 
			ParticleLUP( 2, v, a ) = VertexZ( s, v ) 
		Next
	Next

	;particle mesh
	HideEntity ParticleTemplateMesh
	ParticleMesh = CreateMesh()
	EntityFX ParticleMesh,32+2
	s= CreateSurface( ParticleMesh )
	gs = GetSurface(ParticleTemplateMesh,1)
	cv = CountVertices ( gs )
	ct = CountTriangles ( gs )
	For l= 0 To MaxParticles
		vs= l * cv
		particle(l) = New ParticleType
		particle(l)\Vertex = vs
		For v=0 To cv-1
			v0 = AddVertex(s,0,0,0)
		Next
		For t=0 To ct - 1
			AddTriangle s, TriangleVertex( gs,t,0)+vs,TriangleVertex( gs,t,1)+vs,TriangleVertex( gs,t,2)+vs
		Next 
	Next
		
End Function

;=================================================
;=================================================
;=================================================

Function SOUNDinit()

	s$ = "SoundData2\"
	SOUNDwind = LoadSound( s$+"Wind.wav")
	LoopSound SOUNDwind
	CHANNELwind = PlaySound ( SOUNDwind )
	ChannelVolume CHANNELwind,.25
	PauseChannel CHANNELwind
	SOUNDthunder = LoadSound( s$+"Thunder.wav")
	SOUNDshoot = LoadSound( s$+"Shot.wav");Shoot.wav")
	SOUNDsmall = LoadSound( s$ + "Small.wav")
	SOUNDexplode = LoadSound( s$ + "Explosion.wav") ;Explode.wav")
	SOUNDsplat = LoadSound( s$+"Splat.wav")
	SOUNDlaunch = LoadSound( s$+"Launch.wav")
	SOUNDmenu = LoadSound( s$+"Button.wav")
	SOUNDallie = LoadSound(s$+"DrumRoll.wav")
	SOUNDbabble = LoadSound( s$+"Babble.wav")
	SOUNDfirework = LoadSound( s$+"Firework.wav")
	SOUNDfanfare = LoadSound( s$+"FanFare.wav")
;	SOUNDargh = LoadSound( s$+"Argh.wav")
	SOUNDsplash = LoadSound( s$+"Splash.wav")
		
	;read SoundLoops for 3d sounds
	Restore SOUNDdata
	For l= 0 To SoundGroups
		Read Sound$, pitch
		SoundGroup ( l ) = New SoundGroupType
		SoundGroup ( l )\Sound = LoadSound( s$+Sound$+".wav" )
		SoundGroup(l)\index = l
		LoopSound SoundGroup(l)\Sound
		SoundGroup( l )\channel = PlaySound( SoundGroup(l)\Sound )
		ChannelVolume SoundGroup(l)\channel, 0
		ChannelPitch SoundGroup(l)\channel, pitch
	Next
			
End Function

;=================================================
;=================================================
;=================================================

Function SOUNDon( On )

	If On
		ResumeChannel CHANNELwind
		ResumeChannel CHANNELwave
	Else
		PauseChannel CHANNELwind
		PauseChannel CHANNELwave
		For G.SoundGroupType = Each SoundGroupType
			G\distance = 10000
			G\volume = 0
			ChannelVolume G\channel, 0
		Next
	EndIf

End Function

;=================================================
;=================================================
;=================================================

Function HUDinit()
	
	;hud stuff
	HUD\camera = CreateCamera()
	CameraClsMode HUD\camera,False,True
	PositionEntity HUD\camera,10000,10000,10000
	CameraRange HUD\camera, .1, 1000
	temp = CreatePivot( HUD\camera ) 
	aspect# = Float( GH ) / GW
	scale#=2.0 / GW 
	PositionEntity temp,-1,aspect,1 
	ScaleEntity temp,scale,-scale,-scale
	HUD\pivot = CreatePivot( temp )
	PositionEntity HUD\pivot, -.5,-.5,0
	HUD\model = MESHload("TestHud",0,3, HUD\pivot,1,1)
	HUD\fuel = MESHload("Fuel2",3,2,HUD\pivot,1,1)
	HUD\height = MESHload("Height2",3,2,HUD\pivot,1,1) 
	HUD\bonus = MESHload("Bonus2",3,2,HUD\pivot,1,1) 
	HUD\dot = MESHload("Dot",0,3,0,0,1)
	HUD\Display = TEXTUREload("Hud.bmp")
	HUD\Title = MESHload("Title",0,3,HUD\pivot,1,1)
	TEXTUREsurface( GetSurface(HUD\title, 2 ), 3, 3, TEXTUREload("Title3.bmp"),1 )
	HUD\Map = CreateTexture(size,size,48+256)
	TextureBlend HUD\map,3
	TEXTUREsurface( GetSurface(HUD\model,3) , 3 , 3 , HUD\Display , 0 )
	TEXTUREsurface( GetSurface(HUD\model,2) , 3 , 3 , HUD\Map , 0)
	TEXTUREsurface( GetSurface(HUD\model,4) , 3 , 3 , TEXTUREload("Title2.bmp") , 1)
	HUD\CrossHair = MESHload("Sight",3,0,0,0)
	HUD\arrow = MESHload("Arrow",3,2,0,1,1)
	ScaleEntity HUD\arrow, HGW, HGW, HGW
	EntityOrder HUD\arrow,-1000
	
	HUD\fade = CreateSprite( HUD\camera)
	PositionEntity HUD\fade, 0,0,.5
	EntityColor HUD\fade,0,0,0
	EntityOrder HUD\fade,-5000
	EntityAlpha HUD\fade,.5
	
End Function

;=================================================
;=================================================
;=================================================

Function HUDupdate()

	If player(cam)\fuel > 100 h# = 1 Else h# = ( ( rotate Mod 15 ) / 15.0 )
	ScaleEntity HUD\fuel,player(cam)\fuel / 500.0 * HGW, HGW * h , HGW
	If player(cam)\y < MaxHeight h#=1 Else h# =( ( rotate Mod 15 ) / 15.0 )
	ScaleEntity HUD\height,player(cam)\y / 400.0 *HGW,HGW * h , HGW
	ScaleEntity HUD\bonus,bonustimer / ( FPS*30) *HGW,HGW,HGW

	SetBuffer TextureBuffer(HUD\Display)
	Color 0,0,0
	Rect 0,19,256,11,1
	Color 255,255,255
	Text 16,24,lives,1,1
	Text 56,24,missiles,1,1
	Text 104,24,wave,1,1
	Text 160,24,score,1,1
	Text 224,24,bestscore,1,1
	SetBuffer BackBuffer()
	
End Function

;=================================================
;=================================================
;=================================================

Function TEXTUREsurface( surface, blend, fx, texture, free )

	brush = CreateBrush()
	BrushBlend brush,blend
	BrushFX brush,fx
	BrushTexture brush,texture
	PaintSurface surface,brush
	FreeBrush brush
	If free FreeTexture texture
	
End Function

;=================================================
;=================================================
;=================================================

Function TEXTUREload( file$ )

	file$="MiscData\"+file$
	DebugLog file$
	texture = LoadTexture(file$,48)
	TextureBlend texture, 3
	Return texture

End Function

;=================================================
;=================================================
;=================================================

Function MENUinit()

	Restore MenuData:Read menu_no
	
	For m = 0 To menu_no
	
		menu(m) = New MenuType
		Read menu(m)\min_option
		Read menu(m)\max_option
		Read menu(m)\y_scale
		Read menu(m)\y_spacing
		Read menu(m)\x_spacing
		menu(m)\id = m
			
		menu(m)\texture = CreateTexture(128,menu(m)\y_scale,48)
		TextureBlend menu(m)\texture,3
			
		menu(m)\model = MESHload("MenuBig",0,3,HUD\pivot,0,1)
		ScaleEntity menu(m)\model, HGW, HGW * menu(m)\y_scale / 128.0 , HGW
		EntityOrder menu(m)\Model,-3000
					 
		TEXTUREsurface ( GetSurface(menu(m)\model,2) , 3 , 3 , menu(m)\texture , 0 )
			
		For o = 0 To menu(m)\max_option
			Read menu(m)\name$[o]
			Read menu(m)\sub_min[o]
			
			If menu(m)\sub_min[o] > -1
				Read menu(m)\sub_max[o]
				Read menu(m)\sub_current[o]
				Read menu(m)\sub_name$[o]
			EndIf
			
			If menu(m)\sub_min[o] = -1 
				menu(m)\sub_max[o] = -1
				menu(m)\sub_current[o] = -1
			EndIf
				
		Next
		
	Next	

End Function

;=================================================
;=================================================
;=================================================

Function MENUshow( m.MenuType, ON )
	
	If m = MENU(0)
		m\current = m\max_option
	Else
		m\current = m\min_option
	EndIf
	
	ShowEntity m\model
		
	;fade in		
	SOUNDon( False )
		
	For a# = 0 To 1.0 Step .02:MENUupdate( m, a ):Next
	FlushMouse()
	FlushJoy()
	
	Repeat
	
		;user input
		jx = KeyDown(205) - KeyDown(203)
		jy = KeyDown(208) - KeyDown(200)
		jb = KeyDown(28) Or MouseDown(1) Or JoyDown(1)
		If KeyDown(1) End
		
		;change current option
		If jy <> 0 
			old = m\current
			m\current = LIMIT (m\current + jy, m\min_option, m\max_option)
			jx=0:jy = (old <> m\current)
		EndIf
		
		;change current sub option
		If jx <> 0  
			old = m\sub_current[m\current]
			m\sub_current[m\current] = LIMIT ( m\sub_current[m\current] + jx , m\sub_min[m\current], m\sub_max[m\current] )
			jx = ( old <> m\sub_current[m\current] )
		EndIf
		
		If (jx+jy)<>0 MENUupdate(m)
			
	Until m\sub_min[m\current] = -1 And jb
	
	;fade out 
	For a# = 1.0 To 0.0 Step -.02:MENUupdate( m, a ):Next
	HideEntity m\model
	
	If ON SOUNDon( True )
	
End Function

;=================================================
;=================================================
;=================================================

Function MENUupdate( m.MenuType , scale#=1.0 )

	If ( scale#=0.0 Or scale# = 1.0 )

		SetBuffer TextureBuffer(m\texture)
		
		;clear
		Color 0,0,0:Rect 0,0,128,m\y_scale,1
		
		For o = 0 To m\max_option
		
			sub_menu = m\sub_min[o]
			y=8 + m\y_spacing*o + 4*( o > 0 )
			
			;colour			
			c=125 + (o = m\current) * 130
			Color c,c,c
			If m\sub_max[o] = 0 Color 0,255,255
			If o = 0 Color 255,255,0 
						
			;display option text 
			Text 10+(sub_menu<0)*54,y,m\name$[o],(sub_menu < 0),1
						
			;sub option text
			If sub_menu > -1
				p$ = Mid$(m\sub_name[o],1+m\sub_current[o]*12,12)
				Color c,c*.25,c*.25
				Text m\x_spacing,y,p$,0,1
			EndIf
					
		Next
				
		If scale = 1 PlaySound SOUNDmenu 
		Delay 100
	EndIf

	SetBuffer BackBuffer()
	
	PositionEntity m\model,320,240, ( 1.0 - scale ) * 500
			
	RenderWorld()
	Flip
	
End Function

;=================================================
;=================================================
;=================================================
;=================================================
;=================================================
;=================================================

.SoundData

Data "ThrustLoop", 44000 ;20000
Data "Seeder",80000
Data "Pest",7500
Data "Attractor",30000
Data "Missile",20000
Data "Generator", 10000
Data "Repulsor",7500 
Data "Pest",10000
Data "Monster",10000
Data "Thrust2",5000

.MenuData
Data 4
;min option / max options / yscale / yspacing / xspacing
;option$,sub_min,sub_max,sub_current,sub_name$
;Options
Data 1,5,128,20,64
Data "O P T I O N S",-2	
Data "Difficulty",0,2,1,"Easy        Medium      Hard"
Data "Wave",0,5,0,      "1           5           9           13           17          21 "
Data "Control",0,3,1,   "Demo        Mouse       Original    Joypad      "
Data "Camera",0,2,0,    "Original    Chase       Rear        "
Data "Start",-1
;Wave complete
Data 4,4,128,24,84
Data "WAVE COMPLETE",-2
Data "Infected",0,0,0,""
Data "Un-infected",0,0,0,""
Data "Area Bonus",0,0,0,""
Data "Continue",-1
;game over
Data 1,1,32,12,64
Data "G A M E  O V E R ",-2
Data "OK",-1
;pause
Data 1,2,64,20,64
Data "P A U S E D",-2
Data "Continue",-1
Data "Quit",-1
;game complete
Data 1,1,32,12,64
Data "GAME COMPLETE",-2
Data "Well Done !!!",-1

.WaveData

;				Ho	Se	Bo	Pe	Dr	Mu	Fi	De	At	Re	My	Gd	Gm	Gf	Gd	Ce	El	Mo	Al
;				x4	x1	x1	x1	x1	x1	x1	x1	x1	x1	x4	x4	x4	x4	x4	x13	x4	x1	x1
Data "Level1",	1,	2,	0,	1,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0	;wave1
Data "",			1,	3,	1,	2,	1,	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0	;wave2
Data "",			1,	4,	2,	3,	2,	1,	1,	0,	1,	1,	0,	0,	0,	0,	0,	0,	0,	1,	0	;wave3
Data "",			1,	5,	2,	4,	2,	2,	1,	0,	1,	1,	0,	0,	0,	0,	0,	0,	0,	1,	0	;wave4

Data "Level2",	1,	5,	3,	5,	3,	2,	2,	1,	1,	1,	0,	0,	0,	0,	0,	0,	0,	1,	0	;wave5
Data "",			1,	7,	3,	6,	3,	3,	2,	1,	1,	2,	1,	0,	0,	0,	0,	0,	0,	1,	0	;wave6
Data "",			1,	8,	4,	5,	4,	3,	3,	2,	2,	2,	1,	0,	0,	0,	0,	0,	0,	1,	0	;wave7
Data "",			1,	9,	4,	4,	4,	4,	3,	2,	2,	3,	2,	0,	0,	0,	0,	0,	0,	1,	0	;wave8

Data "Level3",	1,	8,	5,	3,	5,	4,	4,	3,	3,	3,	2,	0,	0,	0,	0,	0,	0,	1,	0	;wave9
Data "",			1,	7,	5,	2,	5,	5,	4,	3,	3,	4,	3,	1,	0,	0,	0,	0,	0,	1,	0	;wave10
Data "",			1,	6,	4,	1,	0,	5,	5,	4,	4,	4,	3,	1,	0,	0,	0,	0,	0,	2,	0	;wave11
Data "",			1,	5,	3,	1,	0,	0,	5,	4,	4,	3,	4,	2,	1,	0,	0,	0,	0,	3,	0	;wave12

Data "Level4",	1,	4,	2,	1,	0,	0,	0,	5,	3,	2,	4,	2,	1,	1,	0,	0,	1,	2,	2	;wave13
Data "",			1,	3,	1,	1,	0,	0,	0,	5,	2,	1,	3,	3,	2,	1,	0,	0,	2,	1,	0	;wave14
Data "",			1,	2,	1,	1,	0,	0,	0,	0,	1,	0,	3,	3,	2,	2,	1,	0,	3,	1,	0	;wave15
Data "",			1,	1,	1,	1,	0,	0,	0,	0,	0,	0,	2,	1,	3,	2,	1,	0,	4,	1,	0	;wave16

Data "Level5",	1,	1,	1,	2,	0,	0,	0,	0,	0,	0,	2,	1,	3,	3,	2,	1,	0,	1,	4	;wave17
Data "",			1,	1,	1,	2,	0,	0,	0,	0,	0,	0,	1,	1,	1,	3,	2,	1,	0,	1,	0	;wave18
Data "",			1,	1,	1,	2,	0,	0,	0,	0,	0,	0,	1,	1,	1,	1,	3,	2,	0,	1,	0	;wave19
Data "",			1,	1,	1,	2,	0,	0,	0,	0,	0,	0,	1,	1,	2,	2,	3,	2,	0,	1,	0	;wave20

Data "Level6",	1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	1,	1,	1,	1,	4,	0,	1,	8	;wave21
Data "",			1,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	1,	0	;wave22

.ParticleTemplateData

;Thrust
Data 5,.75, 250,250,50,250,0,50, 1,2,-.075,.075,-.3,-.2,-.075,.075, 1.0, .025
;Bullet
Data 10,1, 255,255,255,255,255,255,2,2, 0,0,0,0,.5,.5,  0.0, .025
;Missile Trail
Data 5,.75,50,100,50,100,50,100,1,2, -.075,.075,-.075,.075,-.15,-.05, 0.5, .025
;Ground Object Smoke
Data 0,.75,50,100,50,100,50,100, 1,2,-.025, .025, .04,.08,-.025,.025,0,.005
;Water Hit
Data 0,.75,50,50,125,250,250,250,1,2, -.07,.07,.1,.125,-.07,.07, 0.5, .025
;Beach hit
Data 0,.75,125,250,125,250,75,150,1,2, -.06,.06,.075,.1,-.06,.06, 0.5, .025
;Ground Hit
Data 0,.75,75,150,125,250,75,150,1,2, -.07,.07,.075,.1,-.07,.07, 0.5, .025
;LandingPad Hit
Data 0,.75,125,250,125,250,125,250,1,2, -.08,.08,.05,.075,-.08,.08, 0.5, .025
;AlienBuildings Hit
Data 0,.75,250,250,0,250,250,250,1,2, -.08,.08,.05,.075,-.08,.08, 0.5, .025
;rain
Data 3,.75,25,25,100,200,200,200, 1,2, -.025,.025,-.4,-.4,-.025,.025,0.5, .01
;tree infection 
Data 0,.75,250,250,50,100,0,50,1.5,3,-.15,.15,.1,.2,-.15,.15,2,.01
;infected 
Data 0,.75,250,250,50,100,0,50,1.5,3,-.15,.15,.1,.2,-.15,.15,2,.01
;bombs
Data 10,1, 255,255,255,255,255,255,3,3, 0,0,-.2,-.2,0,0,  0.0, .01
;explosion
Data 5,.75,250,250,0,250,0,250,1.5,3,-.15,.15,.25,.4,-.15,.15, 4, .001
;spark
Data 0,.75,75,125,75,125,75,125,1.5,3,-.1,.1,0,.1,-.1,.1, 0, .01
;tractor
Data 0,.75,250,250,0,250,250,250,1,2.5,-.075,.075,-.075,.075,-.05,-.01, 0, .05
;star
Data 0,2,100,200,100,200,100,200,2,2,0,0,0,0,0,0, 0, .025
;too high
Data 0,2,150,250,50,150,150,250,2,2,0,0,0,0,0,0, 0, .025
;cruiser thrust
Data 0,.75, 250,250,0,250,250,250, 2,4,-.25,.25,-.25,.25,-.5,-.3, 0.0, .05
;firework
Data 0,1,128,250,128,250,128,250,1.5,1.5,-.1,.1,.3,.5,-.1,.1, 3, .05
;firework explode
Data 3,.75,250,250,0,250,0,250,1.5,3,-.1,.1,-.1,.1,-.1,.1, 1, .025
;radar
Data 0,.75, 0,250,250,250,0,250, 2,4,-.01,.01,-.01,.01,.2,.2, 0.0, .05


.GameObjectData

;ground objects
Data "Bush","Tree1","Tree2","Tree3","House1","House2","House3","Radar1","Mill1","Rocket","Crate"

;flying objects
Data "Hoverplane" 	;name
Data 0,0				;points
Data 0.02 			;max thrust
Data 0.9925			;momentum
Data 3				;fire rate
Data 15				;missile rate
Data 100			;damage
Data 0				;spin speed
Data 0				;engine type
Data 0				;pitch min
Data 180			;pitch max
Data 5				;turn rate
Data 128			;range
Data 255,255,255		;dot colours
Data 5				;cruise height
Data 0				;soundgroup

Data "Seeder"		;name
Data 100,2			;points
Data 0.001			;max thrust
Data 0.99			;momentum
Data 3				;fire rate
Data 0				;missile rate
Data 500			;damage
Data 5				;spin speed
Data 1				;engine type
Data 0				;pitch min
Data 0				;pitch max
Data 5				;turn rate
Data 0				;range
Data 0,255,255		;dot colours
Data 4
Data 1				;soundgroup

Data "Bomber"		;name
Data 800,9			;points
Data 0.002			;max thrust
Data .99				;momentum
Data 20				;fire rate
Data 0				;missile rate
Data 500			;damage
Data 0				;spin
Data 1				;engine type
Data 0				;pitch min
Data 0				;pitch max
Data 5				;turn rate
Data 0				;range
Data 96,96,255		;dot colours
Data 8
Data 0				;soundgroup

Data "Pest"			;name
Data 400,5			;points
Data 0.04			;max thrust
Data 0.9				;momentum
Data 2				;fire rate
Data 0				;missile rate
Data 500			;damage
Data 10				;spin speed
Data 2				;engine type
Data -75				;pitch min
Data 75				;pitch max
Data 5				;turn rate
Data 0				;range
Data 128,128,128		;dot colours
Data 20
Data 2				;soundgroup

Data "Drone"			;name
Data 300,4			;points
Data 0.015			;max thrust
Data 0.98			;momentum
Data 15				;fire rate
Data 0				;missile rate
Data 500			;damage
Data 0				;spin speed
Data 0				;engine type
Data 0				;pitch min
Data 90				;pitch max
Data 2				;turn rate
Data 48				;range
Data 160,80,40		;dot colours
Data 20
Data 0				;soundgroup

Data "Mutant"		;name
Data 500,6			;points
Data 0.015			;max thrust
Data 0.98			;momentum
Data 12				;fire rate
Data 0				;missile rate
Data 500			;damage
Data 0				;spin speed
Data 0				;engine type
Data 0				;pitch min
Data 90				;pitch max
Data 3				;turn rate
Data 48				;range
Data 255,0,255		;dot colours {new ones}
Data 20
Data 0				;soundgroup

Data "Fighter"		;name
Data 750,7			;points
Data 0.015			;max thrust
Data 0.98			;momentum
Data 9				;fire rate
Data 0				;missile rate
Data 250			;damage
Data 0				;spin speed
Data 0				;engine type
Data 0				;pitch min
Data 90				;pitch max
Data 4				;turn rate
Data 64				;range
Data 255,160,0		;dot colours
Data 20
Data 0				;soundgroup

Data "Destroyer"		;name
Data 2000,11		;points
Data 0.0175			;max thrust
Data .98				;momentum
Data 9				;fire rate
Data 0				;missile rate
Data 50				;damage
Data 0				;spin speed
Data 0				;engine type
Data 0				;pitch min
Data 90				;pitch max
Data 5				;turn rate
Data 80				;range
Data 255,96,96		;dot colours
Data 20
Data 0				;soundgroup

Data "Attractor"		;name
Data 1000,10		;points
Data 0.0015			;max thrust
Data .99				;momentum
Data 2				;fire rate
Data 0				;missile rate
Data 100			;damage
Data 5				;spin speed
Data 1				;engine type
Data 0				;pitch min
Data 0				;pitch max
Data 5				;turn rate
Data 12				;range
Data 255,255,0		;dot colours
Data 5
Data 3				;soundgroup

Data "Repulsor"		;name
Data 1000,10		;points
Data 0.0025			;max thrust
Data .99				;momentum
Data 9				;fire rate
Data 0				;missile rate
Data 100			;damage
Data 15				;spin speed
Data 1				;engine type
Data 0				;pitch min
Data 0				;pitch max
Data 5				;turn rate
Data 15				;range
Data 96,255,96		;dot colours
Data 7.5
Data 6

Data "Mystery"		;name
Data 2000,11		;points
Data 0.0015			;max thrust
Data .99				;momentum
Data 2				;fire rate
Data 150			;missile rate
Data 20				;damage
Data 0				;spin speed
Data 1				;engine type
Data 0				;pitch min
Data 0				;pitch max
Data 5				;turn rate
Data 20				;range
Data 0,0,0			;dot colours
Data 25
Data 0				;soundgroup

Data "Generator"		;name (for drone)
Data 2000,11		;points
Data 0.000001		;max thrust
Data 0.0				;momentum
Data 3				;fire rate
Data 300			;missile rate
Data 10				;damage
Data 2				;spin speed
Data 1				;engine type
Data 0				;pitch min
Data 0				;pitch max
Data 5				;turn rate
Data 64				;range
Data 160,80,40		;dot colours
Data 30
Data 5				;soundgroup

Data "Generator"		;name (for mutant)
Data 2000,11		;points
Data 0.000001		;max thrust
Data 0.0				;momentum
Data 3				;fire rate
Data 150			;missile rate
Data 10				;damage
Data 2				;spin speed
Data 1				;engine type
Data 0				;pitch min
Data 0				;pitch max
Data 5				;turn rate
Data 64				;range
Data 255,0,255		;dot colours {to be done}
Data 30
Data 5				;soundgroup

Data "Generator"		;name (for fighter)
Data 2000,11		;points
Data 0.000001		;max thrust
Data 0.0				;momentum
Data 3				;fire rate
Data 150			;missile rate
Data 10				;damage
Data 2				;spin speed
Data 1				;engine type
Data 0				;pitch min
Data 0				;pitch max
Data 5				;turn rate
Data 64				;range
Data 255,160,0		;dot colours
Data 30
Data 5				;soundgroup

Data "Generator"		;name (for destroyer)
Data 2000,11		;points
Data 0.000001		;max thrust
Data 0.0				;momentum
Data 3				;fire rate
Data 150			;missile rate
Data 10				;damage
Data 2				;spin speed
Data 1				;engine type
Data 0				;pitch min
Data 0				;pitch max
Data 5				;turn rate
Data 64				;range
Data 255,96,96		;dot colours
Data 30
Data 5				;soundgroup

Data "Cruiser"		;name 
Data 5000,13		;points
Data 0.004			;max thrust
Data 0.99			;momentum
Data 3				;fire rate
Data 150			;missile rate
Data 5				;damage
Data 0				;spin speed
Data 3				;engine type
Data 0				;pitch min
Data 0				;pitch max
Data 20				;turn rate
Data 64				;range
Data 192,64,255		;dot colours
Data 50
Data 9				;soundgroup

Data "Elite"			;name
Data 2500,12		;points
Data 0.02			;max thrust
Data 0.9925			;momentum
Data 6				;fire rate
Data 90				;missile rate
Data 25				;damage
Data 0				;spin speed
Data 0				;engine type
Data 0				;pitch min
Data 90				;pitch max
Data 5				;turn rate
Data 128			;range
Data 192,64,255		;dot colours
Data 40
Data 0				;soundgroup

Data "Monster"		;name
Data 2000,11		;points
Data 0.00001		;max thrust
Data 0				;momentum
Data 2				;fire rate
Data 0				;missile rate
Data 500			;damage
Data 1				;spin speed
Data 1				;engine type
Data 0				;pitch min
Data 0				;pitch max
Data 3				;turn rate
Data 32				;range
Data 255,255,255		;dot colours
Data 0
Data 8				;soundgroup

Data "Allie"			;name
Data 0,0				;points
Data 0.02			;max thrust
Data 0.9925			;momentum
Data 3				;fire rate
Data 15				;missile rate
Data 25				;damage
Data 0				;spin speed
Data 0				;engine type
Data 0				;pitch min
Data 180			;pitch max
Data 5				;turn rate
Data 128			;range
Data 255,255,255		;dot colours
Data 20
Data 0				;soundgroup

Data "Spore"			;name
Data 150,3			;points
Data .03				;max thrust
Data .9				;momentum
Data 2				;fire rate
Data 0				;missile rate
Data 500			;damage
Data 20				;spin speed
Data 2				;engine type
Data -75				;pitch min
Data 75				;pitch max
Data 5				;turn rate
Data 0				;range
Data 0,0,0			;dot colour
Data 20
Data 7				;soundgroup

Data "Missile"		;name
Data 0,0				;points
Data .04				;max thrust
Data 0.925			;momentum
Data 2				;fire rate
Data 0				;missile rate
Data 500			;damage
Data 0				;spin speed
Data 2				;engine type
Data -90				;pitch min
Data 90				;pitch max
Data 5				;turn rate
Data 1000			;range
Data 255,255,255		;dot colours
Data 30
Data 4				;soundgroup