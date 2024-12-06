from django.shortcuts import render

# Create your views here.

def start(request):
    return render(request,'index.html')

def go(request):
    if request.method=="POST":
        demo_name=request.POST.get('user_name')
        demo_address=request.POST.get('address')
        demo_college=request.POST.get('college')
        print(demo_name)
        print(demo_address)
        print(demo_college)
        print("hello")

      
        return render(request,'dash.html',{'name':demo_name,'add':demo_address,'clg':demo_college})
    return render(request,'index.html')