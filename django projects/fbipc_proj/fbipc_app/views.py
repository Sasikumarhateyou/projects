from django.shortcuts import render
from.models import register
from django.contrib import messages

# Create your views here.

def start(request):
    return render(request,'log.html')

def log(request):
    if request.method=="POST":
        user=request.POST.get('username')
        password=request.POST.get('password')
        if register.objects.filter(reg_user=user,reg_pass=password).exists():
            return render(request,"dash.html")
        else:
            messages.error(request,"INVALID USERNAME OR PASSWORD")
    return render(request,"log.html")
    
