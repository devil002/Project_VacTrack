# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""

from app.home import blueprint
from flask import render_template, redirect, url_for, request
from flask_login import login_required, current_user
from app import login_manager
from jinja2 import TemplateNotFound

#from app.base.models import user_loader

import sqlite3
from datetime import datetime
from dateutil.tz import gettz
connection = sqlite3.connect('dataofapp/gpsDATA.db')
connection2 = sqlite3.connect('dataofapp/trackerdata.db')
connection3 = sqlite3.connect('dataofapp/gpsDATAc.db')

@blueprint.route('/index')
@login_required
def index():
    pgtuser = str(current_user)
    # for distinct date
    cursor = connection3.cursor()
    cursor.execute("select distinct date from gpsDATA order by date DESC")
    date_list = cursor.fetchall()
    
    #for graph view and all data display
    igraphdata = []
    i = 31
    for date in date_list:
        if date == "" and i == 0 :
            break
        cursor.execute("SELECT COUNT(id) FROM gpsDATA WHERE pgtuser = :pgtuser AND date = :date;",{"pgtuser":pgtuser,"date": date[0]})
        #cursor.execute("SELECT COUNT(id) FROM gpsDATA WHERE date = :date", {"date": date[0]})
        igraphdata.append((date[0],cursor.fetchall()[0][0]))
        #igraphdata[date[0]] = cursor.fetchall()[0][0]
        i -= 1
    
    # latest update on tracker
    cursor.execute("SELECT * FROM (SELECT * FROM gpsDATA WHERE pgtuser = :pgtuser order by id DESC) tmp group by pgtid;",{'pgtuser':pgtuser})
    locatn = cursor.fetchall()
    connection3.commit()
    #cursor.execute("SELECT * FROM gpsDATA WHERE id=(SELECT max(id) FROM gpsDATA)")
    #locatn = cursor.fetchall()
    #connection.commit()
    
    #devices list
    pgtuser = str(current_user)
    deleted = "N"
    cursor = connection2.cursor()
    cursor.execute("select * from trackerdata where pgtuser = :pgtuser and deleted = :deleted order by id DESC;",{'pgtuser':pgtuser,'deleted':deleted})
    device_list = cursor.fetchall()

    cursor.execute("select count(id) from trackerdata where pgtuser = :pgtuser and deleted = :deleted;",{'pgtuser':pgtuser,'deleted':deleted})
    device_all = cursor.fetchall()
    
    status = "Active"
    cursor.execute("select count(id) from trackerdata where pgtuser = :pgtuser and status = :status and deleted = :deleted;",{'pgtuser':pgtuser,'status':status,'deleted':deleted})
    device_active = cursor.fetchall()
    
    status = "Inactive"
    cursor.execute("select count(id) from trackerdata where pgtuser = :pgtuser and status = :status and deleted = :deleted;",{'pgtuser':pgtuser,'status':status,'deleted':deleted})
    device_inactive = cursor.fetchall()
    
    status = "Unknown"
    cursor.execute("select count(id) from trackerdata where pgtuser = :pgtuser and status = :status and deleted = :deleted;",{'pgtuser':pgtuser,'status':status,'deleted':deleted})
    device_unknown = cursor.fetchall()
    
    cursor.execute("SELECT pgtid, status FROM trackerdata WHERE pgtuser = :pgtuser and deleted = :deleted;",{'pgtuser':pgtuser,'deleted':deleted})
    colstat = cursor.fetchall()
    
    connection2.commit()

    return render_template('index.html', segment='index', locatn=locatn, colstat=colstat, device_all=device_all, igraphdata = igraphdata,device_list=device_list, device_active = device_active, device_inactive=device_inactive,device_unknown=device_unknown)

@blueprint.route('/<template>')
@login_required
def route_template(template):

    try:

        if not template.endswith( '.html' ):
            template += '.html'

        # Detect the current page
        segment = get_segment( request )

        # Serve the file (if exists) from app/templates/FILE.html
        return render_template( template, segment=segment )

    except TemplateNotFound:
        return render_template('page-404.html'), 404
    
    except:
        return render_template('page-500.html'), 500

# Helper - Extract current page name from request 
def get_segment( request ): 

    try:

        segment = request.path.split('/')[-1]

        if segment == '':
            segment = 'index'

        return segment    

    except:
        return None  

@blueprint.route('/pages-gwitht')
@login_required
def gwitht():
    pgtuser = str(current_user)
    cursor = connection3.cursor()
    cursor.execute("SELECT * FROM gpsDATA WHERE pgtuser = :pgtuser ORDER BY id DESC;",{'pgtuser':pgtuser})
    data=cursor.fetchall()

    cursor.execute("SELECT * FROM gpsDATA WHERE id=(SELECT max(id) FROM gpsDATA) AND pgtuser = :pgtuser;",{'pgtuser':pgtuser})
    locatn = cursor.fetchall()
    connection.commit()
    return render_template('pages-gwitht.html',segment='pages-gwitht', data=data, locatn=locatn)
    
@blueprint.route('/maps')
@login_required
def maps():
    pgtuser = str(current_user)
    cursor = connection3.cursor()
    #cursor.execute("SELECT * FROM gpsDATA ORDER BY id DESC WHERE user = :user ", user = current_user)
    cursor.execute("SELECT * FROM gpsDATA WHERE pgtuser = :pgtuser ORDER BY id DESC;",{'pgtuser':pgtuser})
    data=cursor.fetchall()

    cursor.execute("SELECT * FROM gpsDATA WHERE id=(SELECT max(id) FROM gpsDATA) AND pgtuser = :pgtuser;",{'pgtuser':pgtuser})
    locatn = cursor.fetchall()
    connection3.commit()
    return render_template('maps.html',segment='maps',locatn=locatn,data=data)
    
@blueprint.route('/pages-blank1', methods=["GET","POST"])
@login_required
def blank1():
    pgtuser = str(current_user)
    
    if request.method == "POST":
       
        cursor = connection3.cursor()
        date = request.form.get("viewgps")
        #cursor.execute("SELECT * FROM gpsDATA ORDER BY id DESC WHERE user = :user ", user = current_user)
        cursor.execute("SELECT * FROM gpsDATA WHERE date = :date AND pgtuser = :pgtuser ORDER BY id DESC;", {"date": date,'pgtuser':pgtuser})
        data=cursor.fetchall()

        #cursor.execute("SELECT * FROM gpsDATA WHERE id=(SELECT max(id) FROM gpsDATA) AND pgtuser = :pgtuser;",{'pgtuser':pgtuser})
        cursor.execute("SELECT * FROM (SELECT * FROM gpsDATA WHERE pgtuser = :pgtuser order by id DESC) tmp group by pgtid;",{'pgtuser':pgtuser})
        locatn = cursor.fetchall()
        connection3.commit()
        
        cursor = connection2.cursor()
        cursor.execute("SELECT pgtid, status FROM trackerdata WHERE pgtuser = :pgtuser;",{'pgtuser':pgtuser})
        colstat = cursor.fetchall()
        connection2.commit()
        
        return render_template("view-map.html",segment='pages-blank1', data=data, locatn=locatn, colstat=colstat, date=request.form.get("viewgps"))
    
    
    cursor = connection3.cursor()
    #cursor.execute("SELECT * FROM gpsDATA ORDER BY id DESC WHERE user = :user ", user = current_user)
    cursor.execute("SELECT * FROM gpsDATA WHERE pgtuser = :pgtuser ORDER BY id DESC;",{'pgtuser':pgtuser})
    data=cursor.fetchall()
    
    cursor.execute("select distinct date from gpsDATA Where pgtuser = :pgtuser order by date DESC",{'pgtuser':pgtuser})
    date_list = cursor.fetchall()
    
    cursor.execute("SELECT * FROM (SELECT * FROM gpsDATA WHERE pgtuser = :pgtuser order by id DESC) tmp group by pgtid;",{'pgtuser':pgtuser})
    locatn = cursor.fetchall()
    connection3.commit()
    
    colstat = connection2.cursor().execute("SELECT pgtid, status FROM trackerdata WHERE pgtuser = :pgtuser;",{'pgtuser':pgtuser}).fetchall()
    connection2.commit()
    return render_template('pages-blank.1.html',segment='pages-blank1', data=data, locatn=locatn, colstat=colstat, date_list = date_list)
    
    
@blueprint.route('/pages-blank2')
@login_required
def blank2():
    cursor = connection.cursor()
    #cursor.execute("SELECT * FROM gpsDATA ORDER BY id DESC WHERE user = :user ", user = current_user)
    cursor.execute("SELECT * FROM gpsDATA ORDER BY id DESC")
    data=cursor.fetchall()
    
    cursor.execute("select distinct date from gpsDATA order by date DESC")
    date_list = cursor.fetchall()
    
    cursor.execute("SELECT * FROM gpsDATA WHERE id=(SELECT max(id) FROM gpsDATA)")
    locatn = cursor.fetchall()
    connection.commit()
    return render_template('pages-blank.2.html',segment='pages-blank2', data=data, locatn=locatn, date_list = date_list)
    
@blueprint.route('/addtracker', methods=["GET","POST"])
@login_required
def addtracker():
    cursor = connection2.cursor()
    pgtuser = str(current_user)
    deleted = "N"
    
    if request.method == "POST": 
        i = 1
        j = 2
        t = datetime.now(tz=gettz('Asia/Kolkata'))
        date = t.strftime("%m/%d/%Y")
        if request.form.get('pgtid') and request.form.get('addpgt'):
            cursor.execute("select * from trackerdata where pgtuser = :pgtuser and deleted = :deleted order by id DESC;",{'pgtuser':pgtuser,'deleted':deleted})
            device_list = cursor.fetchall()
            pgtid = request.form.get('pgtid')
            cursor.execute("select pgtid,deleted from trackerdata where pgtuser = :pgtuser and pgtid = :pgtid;",{'pgtuser':pgtuser,'pgtid':pgtid})
            idfound = cursor.fetchall()
            try:
                if str(request.form.get('pgtid')) in idfound[0]:
                    if idfound[0][1] == "Y":
                        deleted = "N"
                        tag = request.form.get('tagd')
                        cursor.execute("update trackerdata set deleted = :deleted, tag =:tag where pgtid =:pgtid and pgtuser =:pgtuser;",{'deleted':deleted,'tag':tag,'pgtid':pgtid,'pgtuser':pgtuser})
                        return render_template('addtracker.html',segment='addtracker',i=idfound, device_list=device_list)
                    return render_template('addtracker.html',segment='addtracker',i=idfound, device_list=device_list)
            except IndexError:
                pass
            sdate = date
            tag = request.form.get('tagd')
            N = "none"
            deleted = "N"
            status = "Active"
            cursor.execute("insert into trackerdata (pgtuser,pgtid,sdate,edate,status,tag,deleted) values (?,?,?,?,?,?,?);",(str(pgtuser),pgtid,sdate,N,status,tag,deleted,))
            connection2.commit()
            return render_template('addtracker.html',segment='addtracker',i=idfound, device_list=device_list)
        if request.form.get('pgtid') and request.form.get('removepgt'):
            cursor.execute("select * from trackerdata where pgtuser = :pgtuser and deleted = :deleted order by id DESC;",{'pgtuser':pgtuser,'deleted':deleted})
            device_list = cursor.fetchall()
            pgtid = request.form.get('pgtid')
            cursor.execute("select pgtid from trackerdata where pgtuser = :pgtuser and pgtid = :pgtid;",{'pgtuser':pgtuser,'pgtid':pgtid})
            idfound = cursor.fetchall()
            try:
                if str(request.form.get('pgtid')) in idfound[0]:
                    ydeleted = "Y"
                    cursor.execute("update trackerdata set deleted = :deleted where pgtid = :pgtid;",{'deleted':str(ydeleted),'pgtid':pgtid})
                    return render_template('addtracker.html',segment='addtracker',i=idfound, device_list=device_list)
            except IndexError:
                pass
            
            connection2.commit()
            return render_template('addtracker.html',segment="addtracker",i=idfound,device_list=device_list)
        if request.form.get('stat'):
            i = request.form.get('stat').split('_')
            cursor.execute("UPDATE trackerdata SET status = :status where pgtid = :pgtid;",{'status':str(i[1]),'pgtid':str(i[0])})
            cursor.execute("select * from trackerdata where pgtuser = :pgtuser and deleted = :deleted order by id DESC;",{'pgtuser':pgtuser,'deleted':deleted})
            device_list = cursor.fetchall()
            connection2.commit()
            return render_template('addtracker.html',segment='addtracker',i=i, device_list=device_list)
    cursor.execute("select * from trackerdata where pgtuser = :pgtuser and deleted = :deleted order by id DESC;",{'pgtuser':pgtuser,'deleted':deleted})
    device_list = cursor.fetchall()
    connection2.commit()
    return render_template('addtracker.html',segment='addtracker', device_list=device_list)