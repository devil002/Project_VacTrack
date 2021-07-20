# -*- encoding: utf-8 -*-
"""
Copyright (c) 2019 - present AppSeed.us
"""

from flask import jsonify, render_template, redirect, request, url_for
from flask_login import (
    current_user,
    login_required,
    login_user,
    logout_user
)

from app import db, login_manager
from app.base import blueprint
from app.base.forms import LoginForm, CreateAccountForm
from app.base.models import User

from app.base.util import verify_pass

import sqlite3
from datetime import datetime
from dateutil.tz import gettz

connection = sqlite3.connect('dataofapp/gpsDATAc.db')
connection2 = sqlite3.connect('dataofapp/trackerdata.db')

@blueprint.route('/')
def route_default():
    return redirect(url_for('base_blueprint.login'))

## Login & Registration

@blueprint.route('/login', methods=['GET', 'POST'])
def login():
    login_form = LoginForm(request.form)
    if 'login' in request.form:
        
        # read form data
        username = request.form['username']
        password = request.form['password']

        # Locate user
        user = User.query.filter_by(username=username).first()
        
        # Check the password
        if user and verify_pass( password, user.password):

            login_user(user)
            return redirect(url_for('base_blueprint.route_default'))

        # Something (user or pass) is not ok
        return render_template( 'accounts/login.html', msg='Wrong user or password', form=login_form)

    if not current_user.is_authenticated:
        return render_template( 'accounts/login.html',
                                form=login_form)
    return redirect(url_for('home_blueprint.index'))

@blueprint.route('/register', methods=['GET', 'POST'])
def register():
    login_form = LoginForm(request.form)
    create_account_form = CreateAccountForm(request.form)
    if 'register' in request.form:

        username  = request.form['username']
        email     = request.form['email'   ]

        # Check usename exists
        user = User.query.filter_by(username=username).first()
        if user:
            return render_template( 'accounts/register.html', 
                                    msg='Username already registered',
                                    success=False,
                                    form=create_account_form)

        # Check email exists
        user = User.query.filter_by(email=email).first()
        if user:
            return render_template( 'accounts/register.html', 
                                    msg='Email already registered', 
                                    success=False,
                                    form=create_account_form)

        # else we can create the user
        user = User(**request.form)
        db.session.add(user)
        db.session.commit()

        return render_template( 'accounts/register.html', 
                                msg='User created please <a href="/login">login</a>', 
                                success=True,
                                form=create_account_form)

    else:
        return render_template( 'accounts/register.html', form=create_account_form)

@blueprint.route('/logout')
def logout():
    logout_user()
    return redirect(url_for('base_blueprint.login'))

@blueprint.route('/shutdown')
def shutdown():
    func = request.environ.get('werkzeug.server.shutdown')
    if func is None:
        raise RuntimeError('Not running with the Werkzeug Server')
    func()
    return 'Server shutting down...'

## database

@blueprint.route('/inputdata', methods=['GET'])
def query_example():
    
    longitude = request.args.get('longitude')
    latitude = request.args.get('latitude')
    t = datetime.now(tz=gettz('Asia/Kolkata'))
    time = t.strftime("%H:%M:%S")
    date = t.strftime("%m/%d/%Y")
    pgtid = request.args.get('pgtid')
    
    #Still TODO
    # get pgt tracker linked with user 
    cursor = connection2.cursor()
    cursor.execute("select pgtuser from trackerdata where pgtid = :pgtid;",{'pgtid':pgtid})
    pgtuser = cursor.fetchall()
    connection.commit()
    # Verify key is in user keylist
    if pgtuser:
        cursor = connection.cursor()
        #working
        #cursor.execute("INSERT INTO gpsDATA (lngt,latt,time,date) VALUES (?,?,?,?);",(longitude,latitude,time,date))
        cursor.execute("INSERT INTO gpsDATA (pgtid,pgtuser,lngt,latt,time,date) VALUES (?,?,?,?,?,?);",(pgtid,pgtuser[0][0],longitude,latitude,time,date))
        connection.commit() 
        return '''
                    <h1>The longitude value is: {}</h1>
                    <h1>The latitude value is: {}</h1>
                    <h1>The time value is: {}</h1>
                    <h1>The date value is: {}</h1>
                    <h1>The pgtid value is: {}</h1>
                    <h1>The pgtuser value is: {}</h1>
                    '''.format(longitude, latitude, time, date, pgtid, pgtuser[0][0])
    else:
        return '''<h1>404</h1>''', 403

# offline file upload
#@blueprint.route('/fileupload',methods=['GET'])
#def fileupload():
    #hexdata = request.args.get('hexdata')
    #pgtid = request.arg.get('pgtid')
    #foo = request.args.get('ff')
    #if foo == "start":
        #filename = pgtid+'.txt'
        #create txt file
        #append.filename()
    #if foo == 'end':
        #append.filename()
        #create file
        #convert to sql
    #if foo == 'cc':
        #append.filename()
        
    #return  '''
    #            <h1>hexdata: {}</h1>
    #'''.format(hexdata)


## Errors

@login_manager.unauthorized_handler
def unauthorized_handler():
    return render_template('page-403.html'), 403

@blueprint.errorhandler(403)
def access_forbidden(error):
    return render_template('page-403.html'), 403

@blueprint.errorhandler(404)
def not_found_error(error):
    return render_template('page-404.html'), 404

@blueprint.errorhandler(500)
def internal_error(error):
    return render_template('page-500.html'), 500
